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
    static const len_t QUEUE_LEN = 1;
    AugRcvPkt queue[QUEUE_LEN];
    indx_t qFront;

    Ptr<SndPktQ> sndPktQ;
    Ptr<RcvPktQ> rcvPktQ;
    Ptr<CRC> crc;

    void sndAck(Ptr<Pkt>& pkt);

    seq_t minSeqNum;
    bool isValidSeq(seq_t seq);

    atomic<bool> stop;
    Semaphore stopped;
public:
    RcvDataPktQ(Ptr<SndPktQ>& sPQ, Ptr<RcvPktQ>& rPQ, Ptr<CRC>& c);
    void store(); // run in thread
    void collect(Ptr<Pkt>& pkt);
    void stopOp(); 
};

inline RcvDataPktQ::RcvDataPktQ(Ptr<SndPktQ>& sPQ, Ptr<RcvPktQ>& rPQ, Ptr<CRC>& c) :
qFront(0), sndPktQ(sPQ), rcvPktQ(rPQ), minSeqNum(0), 
crc(c), stop(false), stopped(0) {}

inline void RcvDataPktQ::collect(Ptr<Pkt> &pkt) {
    // cout << "Entering RcvDataPktQ::collect() \n";
    // cout.flush();
    queue[qFront].collect(pkt);
    qFront = (qFront+1)%QUEUE_LEN;
    // cout << "Exiting RcvDataPktQ::collect() \n";
    // cout.flush();
}

inline bool RcvDataPktQ::isValidSeq(seq_t seq) {
    // cout << "Entering RcvDataPktQ::isValidSeq() \n";
    // cout.flush();
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
    // cout << "Entering RcvDataPktQ::sndAck() \n";
    // cout.flush();
    Ptr<Pkt> tmpPkt(new Pkt());
    addr_t src, dst;
    seq_t seq;

    pkt->getAddr(dst, src);
    seq = pkt->getSeqNum();

    tmpPkt->setSqnm(seq);
    tmpPkt->setAddr(src, dst);
    tmpPkt->packAck();
    tmpPkt->setCRC(*crc);

    // cout << "RcvDataPktQ::sndAck() storing pkt \n";
    // cout.flush();
    sndPktQ->store(tmpPkt);
    // cout << "RcvDataPktQ::sndAck() stored pkt \n";
    // cout.flush();
    // cout << "Exiting RcvDataPktQ::sndAck() \n";
    // cout.flush();
}

inline void RcvDataPktQ::store() {
    // cout << "Entering RcvDataPktQ::store() \n";
    // cout.flush();

    Ptr<Pkt> tmpPkt;
    seq_t tmpSeq;

    uint64_t count = 0;

    while(true) {
        // cout << "RcvDataPktQ::store() fetching DataPkt \n";
        // cout.flush();

        rcvPktQ->getDataPkt(tmpPkt);

        // cout << "RcvDataPktQ::store() fetched DataPkt \n";
        // cout.flush();

        if(stop == true) {
            break;
        }
        
        // cout << "RcvDataPktQ::store() sending Ack \n";
        // cout.flush();

        sndAck(tmpPkt);

        // cout << "RcvDataPktQ::store() sent Ack \n";
        // cout.flush();

        tmpSeq = tmpPkt->getSeqNum();
        
        if(isValidSeq(tmpSeq)) {
            queue[tmpSeq%QUEUE_LEN].store(tmpPkt);
            if(minSeqNum == tmpSeq) {
                minSeqNum++;
            }
            // cout << "RcvDataPktQ::store() ValidSeq \n";
            // cout.flush();
        }

        count++;
        if(count%1321 == 0) {
            this_thread::yield();
        }
    }

    stopped.signal();
    // cout << "Exiting RcvDataPktQ::store() \n";
    // cout.flush();
}

inline void RcvDataPktQ::stopOp() {
    stop = true;
    queue[minSeqNum].stopOp();
    stopped.wait();
}