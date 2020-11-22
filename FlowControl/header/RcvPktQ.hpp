#pragma once

#include <atomic>
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
    Ptr<PktQ> DataPktQ;
    Ptr<PktQ> AckPktQ;

    Ptr<Receiver> receiver;

    addr_t srcAddr;
    addr_t dstAddr;

    Ptr<CRC> crc;

    void insert(Ptr<Pkt>& pkt);
    void recv();

    thread recvTh;
    atomic<bool> stop;
    Semaphore stopped;
public:
    RcvPktQ(Ptr<Receiver>& rcvr, addr_t src, addr_t dst, Ptr<CRC>& c);
    void getDataPkt(Ptr<Pkt>& pkt);
    void getAckPkt(Ptr<Pkt>& pkt);
    void stopOp();
};

inline void RcvPktQ::recv() {
    Ptr<Pkt> tmp;
    
    while(!stop) {
        receiver->receive(tmp);
        insert(tmp);
    }
    stopped.signal();
}

inline RcvPktQ::RcvPktQ(Ptr<Receiver>& rcvr, addr_t src, addr_t dst, Ptr<CRC>& c): 
receiver(rcvr), srcAddr(src), dstAddr(dst), 
recvTh(&RcvPktQ::recv, this), crc(c),
stop(false), stopped(0) {
    recvTh.detach();
}

inline void RcvPktQ::insert(Ptr<Pkt>& pkt) {
    if(pkt->isCorrupt(*crc)) {
        return;
    }

    addr_t srcTmp, dstTmp;
    pkt->getAddr(srcTmp, dstTmp);

    if(dstTmp != srcAddr || srcTmp != dstAddr) {
        return;
    }

    if (pkt->getType() == Pkt::TYPE_ACK) {
        AckPktQ->push(pkt);
    } else if (pkt->getType() == Pkt::TYPE_DATA) {
        DataPktQ->push(pkt);
    }
}

inline void RcvPktQ::getDataPkt(Ptr<Pkt>& pkt) {
    DataPktQ->pop(pkt);
}

inline void RcvPktQ::getAckPkt(Ptr<Pkt> &pkt) {
    AckPktQ->pop(pkt);
}

inline void RcvPktQ::stopOp() {
    stop = true;
    receiver->stopOp();
    DataPktQ->stopOp();
    AckPktQ->stopOp();
    stopped.wait();
}