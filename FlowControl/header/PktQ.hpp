#pragma once

#include "Semaphore.hpp"
#include "Constant.hpp"
#include "Ptr.hpp"
#include "Pkt.hpp"

class PktQ {
    static const len_t QUEUE_LEN = 64;
    Ptr<Pkt> queue[QUEUE_LEN];
    indx_t rear;
    indx_t front;
    Semaphore mtxPop;
    Semaphore mtxPush;
    Semaphore qEmpty;
    Semaphore qFull;
public:
    PktQ();
    void push(Ptr<Pkt>& pkt);
    void pop(Ptr<Pkt>& pkt);
    void stopOp();
    ~PktQ();
};

inline PktQ::PktQ() :
mtxPop(1), mtxPush(1), qEmpty(0), qFull(QUEUE_LEN) {
    rear = 0;
    front = 0;
}

inline void PktQ::push(Ptr<Pkt>& pkt) {
    qFull.wait();

    mtxPush.wait();
    queue[front] = pkt;
    front = (front + 1)%QUEUE_LEN;
    mtxPush.signal();

    qEmpty.signal();
}

inline void PktQ::pop(Ptr<Pkt>& pkt) {
    qEmpty.wait();

    mtxPop.wait();
    pkt = queue[rear];
    rear = (rear + 1)%QUEUE_LEN;
    mtxPop.signal();

    qFull.signal();
} 

inline void PktQ::stopOp() {
    qEmpty.signal();
    mtxPop.signal();
    qFull.signal();
    mtxPush.signal();
}

inline PktQ::~PktQ() {
    stopOp();
}