#pragma once

#include <atomic>
#include <bits/stdint-uintn.h>
#include <chrono>
#include <thread>

using namespace std;

#include "Pkt.hpp"
#include "Semaphore.hpp"
#include "Ptr.hpp"
#include "SndPktQ.hpp"
#include "Constant.hpp"

class AugSndPkt {
private:
    Ptr<Pkt> pkt;
    Semaphore oldPkt;
    Semaphore mutex;
    bool sent;
    bool acked;
    chrono::high_resolution_clock::time_point start;
    chrono::high_resolution_clock::time_point end;
public:
    AugSndPkt();
    void store(Ptr<Pkt>& p);
    void setAck(atomic<uint64_t>& avgRtt);
    bool send(Ptr<SndPktQ>& sndPktQ);
    void stopOp();
};

inline AugSndPkt::AugSndPkt():
oldPkt(1), mutex(1), sent(true), acked(true) {}

inline void AugSndPkt::store(Ptr<Pkt> &p){
    oldPkt.wait();

    mutex.wait();
    pkt = p;
    sent = false;
    acked = false;
    mutex.signal();
}

inline void AugSndPkt::setAck(atomic<uint64_t>& avgRtt) {
    mutex.wait();
    acked = true;
    mutex.signal();

    oldPkt.signal();

    end = chrono::high_resolution_clock::now();
    auto currVal = chrono::duration_cast<chrono::microseconds>(end - start);
    avgRtt = (avgRtt + currVal.count())/2;
}

inline bool AugSndPkt::send(Ptr<SndPktQ> &sndPktQ) {
    bool isSent = false;
    Ptr<Pkt> tmp;

    mutex.wait();
    if (!acked) {
        tmp = pkt;

        if(!sent) {
            start = chrono::high_resolution_clock::now();
            sent = true;
        }
        isSent = true;
    }
    mutex.signal();

    if(isSent) {
        sndPktQ->store(tmp);
    }

    return isSent;
}

inline void AugSndPkt::stopOp() {
    oldPkt.signal();
    mutex.signal();
    this_thread::yield();
}