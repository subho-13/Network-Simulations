#pragma once

#include <bits/stdint-uintn.h>
#include <thread>
#include <atomic>

using namespace std;

#include "Pkt.hpp"
#include "PktQ.hpp"
#include "Constant.hpp"
#include "Semaphore.hpp"
#include "Sender.hpp"
#include "Ptr.hpp"
#include "CRC.hpp"

class SndPktQ {
    static const len_t QUEUE_LEN = SENDER_WINDOW_LEN*4;
    Ptr<PktQ> pktQ;

    Ptr<Sender> sender;

    addr_t srcAddr;
    addr_t dstAddr;
    Ptr<CRC> crc;

    atomic<bool> stop;
    Semaphore stopped;
public:
    SndPktQ(Ptr<Sender> sndr, addr_t src, addr_t dst, Ptr<CRC>& c);
    void send();
    void store(Ptr<Pkt> pkt);
    void stopOp();
};

inline SndPktQ::SndPktQ(Ptr<Sender> sndr, addr_t src, addr_t dst, Ptr<CRC>& c):
pktQ(new PktQ(QUEUE_LEN)), sender(sndr), 
srcAddr(src), dstAddr(dst), crc(c),
stop(false), stopped(0) {}

inline void SndPktQ::send() {
    Ptr<Pkt> tmp;
    while(true) {
        pktQ->pop(tmp);
        
        if(stop == true) {
            break;
        }

        sender->send(tmp);
        this_thread::yield();
    }

    stopped.signal();
}

inline void SndPktQ::store(Ptr<Pkt> pkt) {
    pkt->setAddr(srcAddr, dstAddr);
    pkt->setCRC(*crc);

    pktQ->push(pkt);
}

inline void SndPktQ::stopOp() {
    stop = true;
    pktQ->stopOp();
    stopped.wait();
    this_thread::yield();
}
