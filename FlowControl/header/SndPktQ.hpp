#pragma once

#include <thread>
#include <atomic>

using namespace std;

#include "Pkt.hpp"
#include "PktQ.hpp"
#include "Constant.hpp"
#include "Semaphore.hpp"
#include "Sender.hpp"
#include "Ptr.hpp"

class SndPktQ {
    Ptr<PktQ> pktQ;
    Ptr<Sender> sender;

    void send();
    atomic<bool> stop;
    Semaphore stopped;
    thread sendTh;
public:
    SndPktQ(Ptr<Sender>& sndr);
    void store(Ptr<Pkt>& pkt);
    void stopOp();
};

inline void SndPktQ::send() {
    Ptr<Pkt> tmp;
    while(true) {
        pktQ->pop(tmp);

        if(stop == true) {
            break;
        }

        sender->send(tmp);
    }

    stopped.signal();
}

inline SndPktQ::SndPktQ(Ptr<Sender>& sndr):
sender(sndr), stop(false), stopped(0),
sendTh(&SndPktQ::send, this) {
    sendTh.detach();
}

inline void SndPktQ::store(Ptr<Pkt>& pkt) {
    pktQ->push(pkt);
}

inline void SndPktQ::stopOp() {
    stop = true;
    pktQ->stopOp();
    stopped.wait();
}