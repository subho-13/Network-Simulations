#pragma once

#include "Constant.hpp"
#include "Ptr.hpp"
#include "UsrChan.hpp"
#include "Pkt.hpp"
#include <thread>

class Receiver {
    Ptr<UsrChan> chan;
    byte_t pktBuf[PACKET_LEN];
public:
    Receiver(Ptr<UsrChan>& chn);
    void receive(Ptr<Pkt>& pkt);
};

inline Receiver::Receiver(Ptr<UsrChan>& chn) : chan(chn) {
    chan->regNewReader();
    this_thread::sleep_for(chrono::seconds(7));
}

inline void Receiver::receive(Ptr<Pkt>& pkt) {
    // cout << "Entering Receiver::receive() \n";
    // cout.flush();
    Ptr<Pkt> newPkt(new Pkt());
    chan->read(pktBuf, PACKET_LEN);
    newPkt->read(pktBuf);
    pkt = newPkt;
    // this_thread::sleep_for(chrono::milliseconds(10));
    // cout << "Exiting Receiver::receive() \n";
    // cout.flush();
}