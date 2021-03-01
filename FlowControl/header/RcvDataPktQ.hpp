#pragma once
#include <thread>
#include <atomic>

using namespace std;

#include "CRC.hpp"
#include "Semaphore.hpp"
#include "Constant.hpp"
#include "RcvPktQ.hpp"
#include "SndPktQ.hpp"
#include "AugRcvPkt.hpp"

class RcvDataPktQ {
    static const len_t QUEUE_LEN = RECEIVER_WINDOW_LEN;
    AugRcvPkt queue[QUEUE_LEN];
    indx_t qFront;

    Ptr<SndPktQ> sndPktQ;
    Ptr<RcvPktQ> rcvPktQ;

    void sndAck(Ptr<Pkt>& pkt);

    seq_t minSeqNum;
    bool isValidSeq(seq_t seq);
    bool isOldSeq(seq_t seq);

    atomic<bool> stop;
    Semaphore stopped;
public:
    RcvDataPktQ(Ptr<SndPktQ>& sPQ, Ptr<RcvPktQ>& rPQ);
    void store();
    void collect(Ptr<Pkt>& pkt);
    void stopOp(); 
};

inline RcvDataPktQ::RcvDataPktQ(Ptr<SndPktQ>& sPQ, Ptr<RcvPktQ>& rPQ) :
qFront(0), sndPktQ(sPQ), rcvPktQ(rPQ), minSeqNum(0), 
stop(false), stopped(0) {}

inline void RcvDataPktQ::collect(Ptr<Pkt>& pkt) {
    queue[qFront].collect(pkt);
    qFront = (qFront+1)%QUEUE_LEN;
}

inline bool RcvDataPktQ::isValidSeq(seq_t seq) {
    if (minSeqNum <= (MAXSEQNO - QUEUE_LEN)) {
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

inline bool RcvDataPktQ::isOldSeq(seq_t seq) {
    if(minSeqNum < QUEUE_LEN) {
        if(seq < minSeqNum || seq >= (minSeqNum - QUEUE_LEN)) {
            return true;
        }
    } else {
        if (seq < minSeqNum && seq >= (minSeqNum - QUEUE_LEN)) {
            return true;
        }
    }

    return false;
}

inline void RcvDataPktQ::sndAck(Ptr<Pkt>& pkt) {
    Ptr<Pkt> tmpPkt(new Pkt());
    seq_t seq = pkt->getSeqNum();
    tmpPkt->setSqnm(seq);
    tmpPkt->packAck();
    sndPktQ->store(tmpPkt);
}

inline void RcvDataPktQ::store() {
    seq_t seq;
    Ptr<Pkt> tmpPkt;
    while(true) {
        rcvPktQ->getDataPkt(tmpPkt);

        if(stop == true) {
            break;
        }

        seq = tmpPkt->getSeqNum();

        if(isValidSeq(seq)) {
            queue[seq%QUEUE_LEN].store(tmpPkt);
            if(minSeqNum == seq) {
                minSeqNum++;
            }
            sndAck(tmpPkt);
        } else if(isOldSeq(seq)) {
            sndAck(tmpPkt);
        }
    }

    stopped.signal();
}

inline void RcvDataPktQ::stopOp() {
    stop = true;
    queue[minSeqNum].stopOp();
    stopped.wait();
    this_thread::yield();
}