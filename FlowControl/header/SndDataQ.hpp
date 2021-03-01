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

    seq_t currSeqNm;
    Ptr<SndDataPktQ> sndDataPktQ;
    atomic<bool> stop;
    Semaphore stopped;
public:
    SndDataQ(Ptr<SndDataPktQ>& sDPQ);
    void send();
    void store(byte_t data[], len_t len);
    void stopOp();
};

inline SndDataQ::SndDataQ(Ptr<SndDataPktQ>& sDPQ):
rear(0), front(0), qLen(0), qEmpty(0), qFull(0),
currSeqNm(0), sndDataPktQ(sDPQ), stop(false), stopped(0) {}

inline void SndDataQ::send() {
    byte_t buffer[Pkt::DATA_LEN];
    len_t tmpLen;
    bool isFullLen = false;

    while(true) {
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
        newPkt->setSqnm(currSeqNm); currSeqNm++;

        sndDataPktQ->store(newPkt);
        this_thread::yield();
    }
    stopped.signal();
}

inline void SndDataQ::store(byte_t data[], len_t len) {
    bool isZeroLen = false;  
    
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
}

inline void SndDataQ::stopOp() {
    stop = true;
    qEmpty.signal();
    qFull.signal();
    stopped.wait();
    this_thread::yield();
}
