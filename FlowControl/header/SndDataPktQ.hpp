#pragma once

#include <thread>
#include <atomic>
using namespace std;

#include "Semaphore.hpp"
#include "SndPktQ.hpp"
#include "Ptr.hpp"
#include "Pkt.hpp"
#include "Constant.hpp"
#include "AugSndPkt.hpp"
#include "RcvPktQ.hpp"

class SndDataPktQ {
    static const len_t QUEUE_LEN = WINDOW_LEN;
    AugSndPkt queue[QUEUE_LEN];
    indx_t qFront;
    Semaphore qEmpty;
    atomic<bool> noPkt;
    
    Ptr<SndPktQ> sndPktQ;
    Ptr<RcvPktQ> rcvPktQ;

    atomic<bool> stop;
    
    void sndData();
    thread sendTh;
    Semaphore sStopped;

    seq_t minSeqNum;

    void rcvAck();
    bool isValidSeq(seq_t seq);
    thread recvTh;
    Semaphore rStopped;
public:
    SndDataPktQ(Ptr<SndPktQ>& sPQ, Ptr<RcvPktQ>& rPQ);
    void store(Ptr<Pkt>& pkt);
    void stopOp();
    ~SndDataPktQ();
};

inline SndDataPktQ::SndDataPktQ(Ptr<SndPktQ>& sPQ, Ptr<RcvPktQ>& rPQ):
sndPktQ(sPQ), rcvPktQ(rPQ), qFront(0), noPkt(true), qEmpty(0), stop(false),
sendTh(&SndDataPktQ::sndData, this), sStopped(0), minSeqNum(0),
recvTh(&SndDataPktQ::rcvAck, this), rStopped(0) {
    sendTh.detach();
    recvTh.detach();
}

inline void SndDataPktQ::sndData() {
    while(true) {
        if(noPkt == true) {
            qEmpty.wait();
        }

        if(stop) {
            break;
        }
        
        bool hasSentAny = false;

        for(indx_t i = 0; i < QUEUE_LEN; i++) {
            hasSentAny |= queue[i].send(sndPktQ);
        }

        noPkt = !hasSentAny;
    }

    sStopped.signal();
}

inline void SndDataPktQ::store(Ptr<Pkt> &pkt) {
    queue[qFront].store(pkt);
    qFront = (qFront + 1)%QUEUE_LEN;

    if(noPkt == true) {
        noPkt = false;
        qEmpty.signal();
    }
}

inline bool SndDataPktQ::isValidSeq(seq_t seq) {
    if (minSeqNum <= MAXSEQNO - sizeof(QUEUE_LEN)) {
        if (seq >= minSeqNum && seq < (minSeqNum + QUEUE_LEN)) {
            return true;
        }
    } else {
        if (seq >= minSeqNum || seq < (minSeqNum + QUEUE_LEN)) {
            return true;
        }
    }

    return false;
}

inline void SndDataPktQ::rcvAck() {
    Ptr<Pkt> tmpPkt;
    seq_t tmpSeq;

    while(!stop) {
        rcvPktQ->getAckPkt(tmpPkt);

        tmpSeq = tmpPkt->getSeqNum();
        if(isValidSeq(tmpSeq)) {
            queue[tmpSeq%QUEUE_LEN].setAck();

            if(minSeqNum == tmpSeq) {
                minSeqNum++;
            }
        }
    }

    rStopped.signal();
}

inline void SndDataPktQ::stopOp() {
    stop = true;
    qEmpty.signal();
    sndPktQ->stopOp();
    rcvPktQ->stopOp();
    sStopped.wait();
    rStopped.wait();
}

inline SndDataPktQ::~SndDataPktQ() {
    stopOp();
}