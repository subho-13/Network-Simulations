#pragma once

#include "Constant.hpp"
#include "Semaphore.hpp"
#include "Ptr.hpp"
#include "Pkt.hpp"
#include <thread>

class AugRcvPkt {
private:
    Ptr<Pkt> pkt;
    Semaphore collected;
    Semaphore newPkt;
public:
    AugRcvPkt();
    void store(Ptr<Pkt>& p);
    void collect(Ptr<Pkt>& p);
    void stopOp();
};

inline AugRcvPkt::AugRcvPkt() : collected(1), newPkt(0) {}

inline void AugRcvPkt::store(Ptr<Pkt> &p) {
    collected.wait();
    pkt = p;
    newPkt.signal();
}

inline void AugRcvPkt::collect(Ptr<Pkt>& p) {
    newPkt.wait();
    p = pkt;
    collected.signal();
}

inline void AugRcvPkt::stopOp() {
    newPkt.signal();
    collected.signal();
    this_thread::yield();
}