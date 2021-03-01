#pragma once

#include "Semaphore.hpp"
#include "Constant.hpp"
#include "Ptr.hpp"
#include "Pkt.hpp"
#include <thread>

class PktQ {
    len_t QUEUE_LEN;
    Ptr<Pkt>* queue;
    indx_t rear;
    indx_t front;
    atomic<indx_t> qLen;

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
QUEUE_LEN(qLen), 
queue(new Ptr<Pkt> [qLen]),
rear(0), front(0) , qLen(0),
qEmpty(0), qFull(qLen), mtxPop(1), mtxPush(1) {}

inline void PktQ::push(Ptr<Pkt>& pkt) {
    if(qLen == QUEUE_LEN) {
        this_thread::yield();
    }
    qFull.wait();

    mtxPush.wait();
    queue[front] = pkt;
    front = (front + 1)%QUEUE_LEN;
    qLen++;
    mtxPush.signal();

    qEmpty.signal();
}

inline void PktQ::pop(Ptr<Pkt>& pkt) {
    if(qLen == 0) {
        this_thread::yield();
    }
    
    qEmpty.wait();

    mtxPop.wait();
    pkt = queue[rear];
    rear = (rear + 1)%QUEUE_LEN;
    qLen--;
    mtxPop.signal();

    qFull.signal();
} 

inline void PktQ::stopOp() {
    qEmpty.signal();
    mtxPop.signal();
    qFull.signal();
    mtxPush.signal();
    this_thread::yield();
}

inline PktQ::~PktQ() {
    delete [] queue;
}