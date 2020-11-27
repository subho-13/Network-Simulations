#pragma once

#include "Semaphore.hpp"
#include "Constant.hpp"
#include "Ptr.hpp"
#include "Pkt.hpp"

class PktQ {
    len_t QUEUE_LEN;
    Ptr<Pkt>* queue;
    indx_t rear;
    indx_t front;

    Semaphore qEmpty;
    Semaphore qFull;
    Semaphore mtxPop;
    Semaphore mtxPush;
public:
    PktQ(len_t qLen);
    void push(Ptr<Pkt>& pkt);
    void pop(Ptr<Pkt>& pkt);
    void stopOp();
    ~PktQ();
};

inline PktQ::PktQ(len_t qLen) :
mtxPop(1), mtxPush(1), qEmpty(0), QUEUE_LEN(qLen), qFull(qLen) {
    rear = 0;
    front = 0;
    queue = new Ptr<Pkt> [qLen];
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
    delete [] queue;
}