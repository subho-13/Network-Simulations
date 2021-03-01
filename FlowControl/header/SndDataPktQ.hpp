#pragma once

#include <bits/stdint-uintn.h>
#include <chrono>
#include <cstdint>
#include <ratio>
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
    static const len_t QUEUE_LEN = SENDER_WINDOW_LEN;

    AugSndPkt queue[QUEUE_LEN];
    indx_t qFront;
    
    Ptr<SndPktQ> sndPktQ;
    Ptr<RcvPktQ> rcvPktQ;

    atomic<uint64_t> rtt;

    atomic<bool> stop;
    Semaphore sStopped;

    seq_t minSeqNum;
    
    bool isValidSeq(seq_t seq);
    Semaphore rStopped;
public:
    SndDataPktQ(Ptr<SndPktQ>& sPQ, Ptr<RcvPktQ>& rPQ);
    void rcvAck();
    void sndData();
    void store(Ptr<Pkt>& pkt);
    void showStat();
    void stopOp();
};

inline SndDataPktQ::SndDataPktQ(Ptr<SndPktQ>& sPQ, Ptr<RcvPktQ>& rPQ):
qFront(0), sndPktQ(sPQ), rcvPktQ(rPQ), rtt(10000), stop(false),
sStopped(0), minSeqNum(0),rStopped(0) {}

inline void SndDataPktQ::sndData() {
    uint64_t waitTime = 1000;
    bool hasSentAny = false;

    while(true) {      
        if(stop) {
            break;
        }
        
        hasSentAny = false;

        for(indx_t i = 0; i < QUEUE_LEN; i++) {
            hasSentAny |= queue[i].send(sndPktQ);
        }

        if(hasSentAny == true) {
            waitTime = rtt*QUEUE_LEN;
            waitTime /= 1000;
            waitTime += 1;
        }
        
        if (rtt < 1000) {
            waitTime *= 5
        }

        if(waitTime < 30) {
            waitTime = 30;
        }

        if(waitTime > 120) {
            waitTime = 120;
        }

        this_thread::sleep_for(chrono::milliseconds(waitTime));
        this_thread::yield();
    }

    sStopped.signal();
}

inline void SndDataPktQ::store(Ptr<Pkt> &pkt) {
    queue[qFront].store(pkt);
    qFront = (qFront + 1)%QUEUE_LEN;
}

inline bool SndDataPktQ::isValidSeq(seq_t seq) {
    if (minSeqNum <= MAXSEQNO - QUEUE_LEN) {
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
            queue[tmpSeq%QUEUE_LEN].setAck(rtt);

            if(minSeqNum == tmpSeq) {
                minSeqNum++;
            }
            
            this_thread::yield();
        }
    }

    rStopped.signal();
}

inline void SndDataPktQ::showStat() {
    cout << "\n RTT is :: " << rtt << " microseconds \n";
    cout.flush();
}

inline void SndDataPktQ::stopOp() {
    stop = true;
    sStopped.wait();
    rStopped.wait();
    this_thread::yield();
}
