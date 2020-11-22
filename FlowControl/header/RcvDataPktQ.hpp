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
    static const len_t QUEUE_LEN = WINDOW_LEN;
    AugRcvPkt queue[QUEUE_LEN];
    indx_t qFront;

    Ptr<SndPktQ> sndPktQ;
    Ptr<RcvPktQ> rcvPktQ;
    Ptr<CRC> crc;

    void sndAck(Ptr<Pkt>& pkt);

    seq_t minSeqNum;
    bool isValidSeq(seq_t seq);

    void store();
    thread storeTh;
    atomic<bool> stop;
    Semaphore stopped;
public:
    RcvDataPktQ(Ptr<SndPktQ>& sPQ, Ptr<RcvPktQ>& rPQ, Ptr<CRC>& c);
    void collect(Ptr<Pkt>& pkt);
    void stopOp(); 
};

inline RcvDataPktQ::RcvDataPktQ(Ptr<SndPktQ>& sPQ, Ptr<RcvPktQ>& rPQ, Ptr<CRC>& c) :
qFront(0), sndPktQ(sPQ), rcvPktQ(rPQ), minSeqNum(0), crc(c),
storeTh(&RcvDataPktQ::store, this), stop(false), stopped(0) {
    storeTh.detach();
}

inline void RcvDataPktQ::collect(Ptr<Pkt> &pkt) {
    queue[qFront].collect(pkt);
    qFront = (qFront+1)%QUEUE_LEN;
}

inline bool RcvDataPktQ::isValidSeq(seq_t seq) {
    if (minSeqNum < (MAXSEQNO - sizeof(QUEUE_LEN) + 1)) {
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

inline void RcvDataPktQ::sndAck(Ptr<Pkt>& pkt) {
    Ptr<Pkt> tmpPkt(new Pkt());
    addr_t src, dst;
    seq_t seq;

    pkt->getAddr(dst, src);
    seq = pkt->getSeqNum();

    tmpPkt->setSqnm(seq);
    tmpPkt->setAddr(src, dst);
    tmpPkt->packAck();
    tmpPkt->setCRC(*crc);

    sndPktQ->store(tmpPkt);
}

inline void RcvDataPktQ::store() {
    Ptr<Pkt> tmpPkt;
    seq_t tmpSeq;

    while(true) {
        rcvPktQ->getDataPkt(tmpPkt);

        if(stop == true) {
            break;
        }

        sndAck(tmpPkt);

        tmpSeq = tmpPkt->getSeqNum();
        if(isValidSeq(tmpSeq)) {
            queue[tmpSeq%QUEUE_LEN].store(tmpPkt);
            if(minSeqNum == tmpSeq) {
                minSeqNum++;
            }
        }

    }

    stopped.signal();
}

inline void RcvDataPktQ::stopOp() {
    stop = true;
    queue[minSeqNum].stopOp();
    stopped.wait();
}