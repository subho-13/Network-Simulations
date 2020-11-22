#pragma once

#include "Constant.hpp"
#include "Semaphore.hpp"
#include "Ptr.hpp"
#include "Pkt.hpp"

class AugRcvPkt {
private:
    Ptr<Pkt> pkt;
    Semaphore newPkt;
    Semaphore collected;
public:
    AugRcvPkt();
    void store(Ptr<Pkt>& p);
    void collect(Ptr<Pkt>& p);
    void stopOp();
    ~AugRcvPkt();
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
}

inline AugRcvPkt::~AugRcvPkt() {
    stopOp();
}

/*
AugRcvPkt - Store received DataPackets
store and collect will work in different threads
Semaphores  
1. newPkt(0) (is this a new packet)
2. collected(1) (is the packet collected)

store:
    collected.wait() // is the old packet collected?
    store packet
    newPkt.signal() // notify that a newPkt is present

collect:
    newPkt.wait()   // is a new packet available?
    collect packet
    collected.signal() // notify that the old packet 
                       //is collected

When destructor is called, get done with the semaphores

checked? Yes
*/