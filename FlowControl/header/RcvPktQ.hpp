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

#define DEBUG 1

using namespace std;

class RcvPktQ {
    Ptr<PktQ> DataPktQ;
    Ptr<PktQ> AckPktQ;

    Ptr<Receiver> receiver;

    addr_t srcAddr;
    addr_t dstAddr;

    Ptr<CRC> crc;

    Ptr<atomic<uint64_t>> numPackets;

    atomic<bool> stop;
    Semaphore stopped;
public:
    RcvPktQ(Ptr<Receiver>& rcvr, addr_t src, addr_t dst, Ptr<CRC>& c, Ptr<atomic<uint64_t>>& n);
    void recv(); // Run in thread
    void getDataPkt(Ptr<Pkt>& pkt);
    void getAckPkt(Ptr<Pkt>& pkt);
    void stopOp();
};

inline void RcvPktQ::recv() {
    // cout << "Entering RcvPktQ::recv() \n";
    // cout.flush();

    Ptr<Pkt> pkt(new Pkt());

    uint64_t count = 0;
    
    while(!stop) {
        count++;

        if(count%1723 == 0) {
            this_thread::yield();
        }
        // cout << "RcvPktQ::recv() receiving\n";
        // cout.flush();
        receiver->receive(pkt);
        // cout << "RcvPktQ::recv() received\n";
        // cout.flush();
        // cout << "RcvPktQ::recv() inserting\n";
        // cout.flush();
        // cout << "Entering RcvPktQ::insert() \n";
        // cout.flush();

        if(pkt->isCorrupt(*crc)) {
            // cout << "Entering RcvPktQ::insert(), packet corrupt \n";
            // cout.flush();
            continue;
        }

        addr_t srcTmp, dstTmp;
        pkt->getAddr(srcTmp, dstTmp);

        if(dstTmp != srcAddr || srcTmp != dstAddr) {
            // cout << "Entering RcvPktQ::insert(), incorrect address \n";
            // cout.flush();

            continue;
        }

        (*numPackets)++;
        
        if (pkt->getType() == Pkt::TYPE_ACK) {
            // cout << "Entering RcvPktQ::insert(), insering Ack \n";
            // cout.flush();
            AckPktQ->push(pkt);
        } else if (pkt->getType() == Pkt::TYPE_DATA) {
            // cout << "Entering RcvPktQ::insert(), inserting Data \n";
            // cout.flush();
            DataPktQ->push(pkt);
        }
        // cout << "RcvPktQ::recv() inserted\n";
        // cout.flush();
        // this_thread::sleep_for(chrono::milliseconds(20));
        // this_thread::yield();
    }
    stopped.signal();

    // cout << "Exiting RcvPktQ::recv() \n";
    // cout.flush();
}

inline RcvPktQ::RcvPktQ(Ptr<Receiver>& rcvr, addr_t src, addr_t dst, Ptr<CRC>& c, Ptr<atomic<uint64_t>>& n): 
receiver(rcvr), srcAddr(src), dstAddr(dst), 
crc(c), stop(false), stopped(0), AckPktQ(new PktQ(WINDOW_LEN*3)),
DataPktQ(new PktQ(WINDOW_LEN*3)), numPackets(n) {}

inline void RcvPktQ::getDataPkt(Ptr<Pkt>& pkt) {
    // cout << "Entering RcvPktQ::getDataPktQ() \n";
    // cout.flush();

    DataPktQ->pop(pkt);

    // cout << "Exiting RcvPktQ::getDataPktQ() \n";
    // cout.flush();
}

inline void RcvPktQ::getAckPkt(Ptr<Pkt> &pkt) {
    // cout << "Entering RcvPktQ::getAckPktQ() \n";
    // cout.flush();

    AckPktQ->pop(pkt);

    // cout << "Exiting RcvPktQ::getDataPktQ() \n";
    // cout.flush();
}

inline void RcvPktQ::stopOp() {
    stop = true;
    DataPktQ->stopOp();
    AckPktQ->stopOp();
    stopped.wait();
}