#pragma once

#include <thread>
#include <atomic>

using namespace std;

#include "Pkt.hpp"
#include "PktQ.hpp"
#include "Constant.hpp"
#include "Semaphore.hpp"
#include "Sender.hpp"
#include "Ptr.hpp"

class SndPktQ {
    Ptr<PktQ> pktQ;
    Ptr<Sender> sender;    
    atomic<bool> stop;
    Semaphore stopped;
public:
    SndPktQ(Ptr<Sender>& sndr);
    void send();
    void store(Ptr<Pkt>& pkt);
    void stopOp();
};

inline void SndPktQ::send() {
    // cout << "Entering SndPktQ::send() \n";
    // cout.flush();
    Ptr<Pkt> tmp;
    uint64_t count = 0;
    while(true) {
        count++;

        if(count%1129 == 0) {
            this_thread::yield();
        }
        // cout << "SndPktQ::send() popping pkt \n";
        // cout.flush();
        pktQ->pop(tmp);
        // cout << "SndPktQ::send() popped pkt \n";
        // cout.flush();
        if(stop == true) {
            break;
        }

        // cout << "SndPktQ::send() sending pkt \n";
        // cout.flush();
        sender->send(tmp);
        // cout << "SndPktQ::send() sent pkt \n";
        // cout.flush();
    }

    stopped.signal();
    // cout << "Exiting SndPktQ::send() \n";
    // cout.flush();
}

inline SndPktQ::SndPktQ(Ptr<Sender>& sndr):
pktQ(new PktQ(WINDOW_LEN*3)), sender(sndr), stop(false), stopped(0) {}

inline void SndPktQ::store(Ptr<Pkt>& pkt) {
    // cout << "Entering SndPktQ::store() \n";
    // cout.flush();
    pktQ->push(pkt);
    // cout << "Exiting SndPktQ::store() \n";
    // cout.flush();
}

inline void SndPktQ::stopOp() {
    stop = true;
    pktQ->stopOp();
    stopped.wait();
}