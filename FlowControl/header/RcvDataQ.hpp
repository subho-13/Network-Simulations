#pragma once

#include <atomic>
#include <thread>

using namespace std;

#include "Constant.hpp"
#include "Semaphore.hpp"
#include "RcvDataPktQ.hpp"

class RcvDataQ {
private:
    static const len_t QUEUE_LEN = PACKET_LEN*WINDOW_LEN;
    byte_t queue[QUEUE_LEN];
    indx_t rear;
    indx_t front;

    Ptr<RcvDataPktQ> rcvDataPktQ;

    Semaphore qFull;
    Semaphore qEmpty;

    void store();
    thread storeTh;
    atomic<bool> stop;
    Semaphore stopped;
public:
    RcvDataQ(Ptr<RcvDataPktQ>& rDPQ);
    void collect(byte_t data[], len_t len);
    void stopOp();
};

inline RcvDataQ::RcvDataQ(Ptr<RcvDataPktQ>& rDPQ):
rcvDataPktQ(rDPQ), rear(0), front(0), qFull(QUEUE_LEN), 
qEmpty(0), storeTh(&RcvDataQ::store, this), 
stop(false), stopped(0){
    storeTh.detach();
}

inline void RcvDataQ::collect(byte_t data[], len_t len) {
    for(indx_t i = 0; i < len; i++) {
        qEmpty.wait();
        data[i] = queue[rear];
        rear = (rear+1)%QUEUE_LEN;
        qFull.signal();
    }
}

inline void RcvDataQ::store() {
    byte_t buffer[PACKET_LEN];
    len_t len;
    Ptr<Pkt> tmpPkt;

    while(!stop) {
        rcvDataPktQ->collect(tmpPkt);

        if(stop == true) {
            break;
        }

        tmpPkt->unpackData(buffer, len);
        for(indx_t i = 0; i < len; i++) {
            qFull.wait();
            queue[front] = buffer[i];
            front = (front+1)%QUEUE_LEN;
            qEmpty.signal();

            if(stop == true) {
                break;
            }
        }
    }
    stopped.signal();
}

inline void RcvDataQ::stopOp() {
    stop = true;
    qFull.signal();
    stopped.wait();
}