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
    static const len_t QUEUE_LEN = 1;
    AugSndPkt queue[QUEUE_LEN];
    indx_t qFront;
    
    Ptr<SndPktQ> sndPktQ;
    Ptr<RcvPktQ> rcvPktQ;

    Ptr<atomic<uint64_t>> avgRtt;
    atomic<uint64_t> waitTime;

    atomic<bool> stop;
    Semaphore sStopped;

    seq_t minSeqNum;
    
    bool isValidSeq(seq_t seq);
    Semaphore rStopped;
public:
    SndDataPktQ(Ptr<SndPktQ>& sPQ, Ptr<RcvPktQ>& rPQ, Ptr<atomic<uint64_t>>& avgR);
    void rcvAck();
    void sndData();
    void store(Ptr<Pkt>& pkt);
    void stopOp();
};

inline SndDataPktQ::SndDataPktQ(Ptr<SndPktQ>& sPQ, Ptr<RcvPktQ>& rPQ, Ptr<atomic<uint64_t>>& avgR):
sndPktQ(sPQ), rcvPktQ(rPQ), qFront(0), stop(false),
sStopped(0), minSeqNum(0),rStopped(0), avgRtt(avgR),
waitTime(200000) {}

inline void SndDataPktQ::sndData() {
    // cout << "Entering SndDataPktQ::sndData() \n";
    // cout.flush();    

    while(true) {
        if(stop) {
            break;
        }
        
        bool hasSentAny = false;

        for(indx_t i = 0; i < QUEUE_LEN; i++) {
            // cout << "SndDataPktQ::sndData() sending "<< i <<" packet \n";
            // cout.flush(); 
            hasSentAny |= queue[i].send(sndPktQ);
            // cout << "SndDataPktQ::sndData() sent "<< i <<" packet \n";
            // cout.flush(); 
        }

        waitTime = (*avgRtt)*(QUEUE_LEN)/1000 + 1;

        if(waitTime < 29) {
            waitTime = 29;
        }

        if(waitTime > 2000) {
            waitTime = 2000;
        }

        this_thread::sleep_for(chrono::milliseconds(waitTime));     
    }

    sStopped.signal();
    // cout << "Exiting SndDataPktQ::sndData() \n";
    // cout.flush(); 
}

inline void SndDataPktQ::store(Ptr<Pkt> &pkt) {
    // cout << "Entering SndDataPktQ::store() \n";
    // cout.flush();
    queue[qFront].store(pkt);
    qFront = (qFront + 1)%QUEUE_LEN;

    // cout << "Exiting SndDataPktQ::store() \n";
    // cout.flush();
}

inline bool SndDataPktQ::isValidSeq(seq_t seq) {
    // cout << "Entering SndDataPktQ::isValidSeq() \n";
    // cout.flush();
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
    // cout << "Entering SndDataPktQ::rcvAck() \n";
    // cout.flush();
    Ptr<Pkt> tmpPkt;
    seq_t tmpSeq;
    uint64_t count = 0;

    while(!stop) {
        // cout << "SndDataPktQ::rcvAck() fetching Ack\n";
        // cout.flush();
        rcvPktQ->getAckPkt(tmpPkt);
        // cout << "SndDataPktQ::rcvAck() fetched Ack\n";
        // cout.flush();
        tmpSeq = tmpPkt->getSeqNum();
        if(isValidSeq(tmpSeq)) {
            queue[tmpSeq%QUEUE_LEN].setAck(avgRtt);

            if(minSeqNum == tmpSeq) {
                minSeqNum++;
            }
        }

        count++;

        if(count % 1417 == 0) {
            this_thread::yield();
        }
    }

    rStopped.signal();
}

inline void SndDataPktQ::stopOp() {
    stop = true;
    sStopped.wait();
    rStopped.wait();
}