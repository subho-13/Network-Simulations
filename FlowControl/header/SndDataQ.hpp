#pragma once

#include "semaphore.h"
#include "thread"
#include "atomic"

#include "Ptr.hpp"
#include "Pkt.hpp"
#include "Constant.hpp"
#include "SndDataPktQ.hpp"
#include "CRC.hpp"
#include "Semaphore.hpp"
#include <bits/stdint-uintn.h>
#include <chrono>

using namespace std;

class SndDataQ{
    static const len_t QUEUE_LEN = PACKET_LEN*64;
    byte_t queue[QUEUE_LEN];
    indx_t rear;
    indx_t front;

    atomic<len_t>  qLen;
    Semaphore qEmpty;
    Semaphore qFull;

    addr_t srcAddr;
    addr_t dstAddr;
    seq_t currSeqNm;
    Ptr<CRC> crc;
    Ptr<SndDataPktQ> sndDataPktQ;
    atomic<bool> stop;
    Semaphore stopped;
public:
    SndDataQ(Ptr<SndDataPktQ>& sDPQ, Ptr<CRC>& c, addr_t src,addr_t dst);
    void send(); // run in thread
    void store(byte_t data[], len_t len);
    void stopOp();
};

inline SndDataQ::SndDataQ(Ptr<SndDataPktQ>& sDPQ, Ptr<CRC>& c, addr_t src,addr_t dst):
rear(0), front(0), qLen(0), qEmpty(0), qFull(0),
srcAddr(src), dstAddr(dst), currSeqNm(0), crc(c),
sndDataPktQ(sDPQ), stop(false), stopped(0) {}

inline void SndDataQ::send() {
    // cout << "Entering SndDataQ::send() \n";
    // cout.flush();

    byte_t buffer[Pkt::DATA_LEN];
    len_t tmpLen;
    bool isFullLen = false;

    uint64_t count = 0;

    while(true) {
        count++;
        if (qLen == 0) {
            this_thread::yield();
            qEmpty.wait();
        }

        if(stop) {
            break;
        }

        tmpLen = qLen;

        if(tmpLen > Pkt::DATA_LEN) {
            tmpLen = Pkt::DATA_LEN;
        }

        for(indx_t i = 0; i < tmpLen; i++) {
            buffer[i] = queue[rear];
            rear = (rear + 1)%QUEUE_LEN;

            if (qLen == QUEUE_LEN) {
                isFullLen = true;
            }

            qLen--;

            if(isFullLen) {
                qFull.signal();
                isFullLen = false;
            }
        }

        Ptr<Pkt> newPkt(new Pkt());
        newPkt->packData(buffer, tmpLen);
        newPkt->setAddr(srcAddr, dstAddr);
        newPkt->setSqnm(currSeqNm); currSeqNm++;
        newPkt->setCRC(*crc);

        // cout << "SndDataQ::send() storing pkt \n";
        // cout.flush();
        sndDataPktQ->store(newPkt);
        // cout << "SndDataQ::send() stored pkt \n";
        // cout.flush();
        if(count%1317 == 0) {
            this_thread::yield();
        }
        // this_thread::sleep_for(chrono::milliseconds(10));
    }

    stopped.signal();

    // cout << "Exiting SndDataQ::send() \n";
    // cout.flush();
}

inline void SndDataQ::store(byte_t data[], len_t len) {
    bool isZeroLen = false;

    // cout << "Entering SndDataQ::store() \n";
    // cout.flush();    
    
    for(indx_t i = 0; i < len; i++) {
        if (qLen == QUEUE_LEN) {
            qFull.wait();
        }

        queue[front] = data[i];
        front = (front + 1)%QUEUE_LEN;

        if (qLen == 0) {
            isZeroLen = true;
        }

        qLen++;

        if(isZeroLen) {
            qEmpty.signal();
            isZeroLen = false;
        }
    }
    // cout << "Exiting SndDataQ::store() \n";
    // cout.flush();
}

inline void SndDataQ::stopOp() {
    stop = true;
    qEmpty.signal();
    qFull.signal();
    stopped.wait();
}