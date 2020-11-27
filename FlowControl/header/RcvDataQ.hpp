#pragma once

#include <atomic>
#include <thread>

using namespace std;

#include "Constant.hpp"
#include "Semaphore.hpp"
#include "RcvDataPktQ.hpp"

#define DEBUG 1

class RcvDataQ {
private:
    static const len_t QUEUE_LEN = PACKET_LEN*WINDOW_LEN;
    byte_t queue[QUEUE_LEN];
    indx_t rear;
    indx_t front;

    Ptr<RcvDataPktQ> rcvDataPktQ;

    Semaphore qFull;
    Semaphore qEmpty;
    
    atomic<bool> stop;
    Semaphore stopped;
public:
    RcvDataQ(Ptr<RcvDataPktQ>& rDPQ);
    void store(); // run in thread
    void collect(byte_t data[], len_t len);
    void stopOp();
};

inline RcvDataQ::RcvDataQ(Ptr<RcvDataPktQ>& rDPQ):
rcvDataPktQ(rDPQ), rear(0), front(0), qFull(QUEUE_LEN), 
qEmpty(0), stop(false), stopped(0){}

inline void RcvDataQ::collect(byte_t data[], len_t len) {
    // cout << "Entering RcvDataQ::collect() \n";
    // cout.flush();
    for(indx_t i = 0; i < len; i++) {
        qEmpty.wait();
        
        data[i] = queue[rear];
        rear = (rear+1)%QUEUE_LEN;

        qFull.signal();
    }
    // cout << "Exiting RcvDataQ::collect() \n";
    // cout.flush();
}

inline void RcvDataQ::store() {
    // cout << "Entering RcvDataQ::store() \n";
    // cout.flush();

    byte_t buffer[PACKET_LEN];
    len_t len;
    Ptr<Pkt> tmpPkt;

    uint64_t count = 0;

    while(true) {
        // cout << "RcvDataQ::store() collecting pkt\n";
        // cout.flush();
        rcvDataPktQ->collect(tmpPkt);
        // cout << "RcvDataQ::store() collected pkt\n";
        // cout.flush();

        if(stop == true) {
            break;
        }

        tmpPkt->unpackData(buffer, len);
        // cout << "RcvDataQ::store() inserting data\n";
        // cout.flush();
        for(indx_t i = 0; i < len; i++) {
            qFull.wait();
            queue[front] = buffer[i];
            front = (front+1)%QUEUE_LEN;
            qEmpty.signal();

            if(stop == true) {
                break;
            }
        }

        count++;
        // this_thread::sleep_for(chrono::milliseconds(11)*2);
        if(count == QUEUE_LEN/2) {
            this_thread::yield();
        }
        // cout << "RcvDataQ::store() inserted data\n";
        // cout.flush();
    }
    
    stopped.signal();
    // cout << "Exiting RcvDataQ::store() \n";
    // cout.flush();
}

inline void RcvDataQ::stopOp() {
    stop = true;
    qFull.signal();
    stopped.wait();
}