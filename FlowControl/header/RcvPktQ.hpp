#pragma once

#include <atomic>
#include <bits/stdint-uintn.h>
#include <chrono>
#include <thread>

using namespace std;

#include "Semaphore.hpp"
#include "Receiver.hpp"
#include "Ptr.hpp"
#include "Pkt.hpp"
#include "Constant.hpp"
#include "PktQ.hpp"
#include "CRC.hpp"

using namespace std;

class RcvPktQ {
    static const len_t QUEUE_LEN = RECEIVER_WINDOW_LEN*4;

    Ptr<PktQ> AckPktQ;
    Ptr<PktQ> DataPktQ;

    Ptr<Receiver> receiver;

    addr_t srcAddr;
    addr_t dstAddr;
    Ptr<CRC> crc;

    atomic<uint64_t> numValidPackets;
    atomic<uint64_t> numPackets;

    chrono::high_resolution_clock::time_point start;
    chrono::high_resolution_clock::time_point current;
    atomic<bool> stop;
    Semaphore stopped;
public:
    RcvPktQ(Ptr<Receiver>& rcvr, addr_t src, addr_t dst, Ptr<CRC>& c);
    void recv();
    void getDataPkt(Ptr<Pkt>& pkt);
    void getAckPkt(Ptr<Pkt>& pkt);
    void showStat();
    void stopOp();
};

inline RcvPktQ::RcvPktQ(Ptr<Receiver>& rcvr, addr_t src, addr_t dst, Ptr<CRC>& c): 
AckPktQ(new PktQ(QUEUE_LEN)), DataPktQ(new PktQ(QUEUE_LEN)),
receiver(rcvr),
 srcAddr(src), dstAddr(dst), crc(c), 
 stop(false), stopped(0) {}

inline void RcvPktQ::recv() {
    Ptr<Pkt> pkt(new Pkt());
    addr_t src, dst;
    
    start = chrono::high_resolution_clock::now();
    while(!stop) {        
        receiver->receive(pkt);

        numPackets++;

        if(pkt->isCorrupt(*crc)) {
            continue;
        }

        pkt->getAddr(src, dst);

        if(dst != srcAddr || src != dstAddr) {
            continue;
        }
        
        if (pkt->getType() == Pkt::TYPE_ACK) {
            AckPktQ->push(pkt);
        } else if (pkt->getType() == Pkt::TYPE_DATA) {
            DataPktQ->push(pkt);
        }

        numValidPackets++;
        this_thread::yield();
    }
    
    stopped.signal();
}

inline void RcvPktQ::getDataPkt(Ptr<Pkt>& pkt) {
    DataPktQ->pop(pkt);
}

inline void RcvPktQ::getAckPkt(Ptr<Pkt> &pkt) {
    AckPktQ->pop(pkt);
}


inline void RcvPktQ::showStat() {
    current = chrono::high_resolution_clock::now();
    cout << numValidPackets << " valid packets out of "
         << numPackets << " packets received in "
         << chrono::duration_cast<chrono::milliseconds>(current-start).count()
         << " milliseconds \n";
    cout.flush();
}

inline void RcvPktQ::stopOp() {
    stop = true;
    DataPktQ->stopOp();
    AckPktQ->stopOp();
    stopped.wait();
    this_thread::yield();
}
