#pragma once

#include <atomic>

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
    indx_t retry;
public:
    AugSndPkt();
    void store(Ptr<Pkt>& p);
    void setAck();
    bool send(Ptr<SndPktQ>& sndPktQ);
    void stopOp();
    ~AugSndPkt();
};

inline AugSndPkt::AugSndPkt():
oldPkt(1), mutex(1), retry(0), sent(true), acked(true) {}

inline void AugSndPkt::store(Ptr<Pkt> &p){
    oldPkt.wait();

    mutex.wait();
    pkt = p;
    sent = false;
    acked = false;
    retry = 0;
    mutex.signal();
}

inline void AugSndPkt::setAck() {
    mutex.wait();
    acked = true;
    mutex.signal();

    oldPkt.signal();
}

inline bool AugSndPkt::send(Ptr<SndPktQ> &sndPktQ) {
    bool isSent = false;

    mutex.wait();
    if (!acked) {
        sndPktQ->store(pkt);

        if(sent) {
            retry++;
        } else {
            sent = true;
        }
        isSent = true;
    }
    mutex.signal();

    return isSent;
}

inline void AugSndPkt::stopOp() {
    oldPkt.signal();
    mutex.signal();
}

inline AugSndPkt::~AugSndPkt() {
    stopOp();
}



/*
AugSndPkt - Store data packets to store
sent, acked - Conditions
Semaphores - 
oldPkt(1) - Locks if the old packet is not acked
mutex(1) - Locks to update sent/acked
retry - Use in later purposes

store:
    oldPkt.wait() // wait for the old packet to be acked
    store packet

    mutex.lock() // update sent, acked and retry
    sent, acked = false
    retry = 0
    mutex.unlock()

send:
    check if it's already acked
    if so, do nothing (use mutex), return false (nothing sent)

    else send the packet to sndPktQ

    now again, if it was already sent, increment retry
    else set sent = true
    return true (sent something)

setAck:
    use mutex to set acked = true
    signal intake of new packet

Use destructor to get done with any semaphore
*/