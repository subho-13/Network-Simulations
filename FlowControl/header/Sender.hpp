#pragma once

#include "Constant.hpp"
#include "Ptr.hpp"
#include "UsrChan.hpp"
#include "Pkt.hpp"

class Sender {
    Ptr<UsrChan> chan;
    byte_t pktBuf[PACKET_LEN];
public:
    Sender(Ptr<UsrChan>& chn);
    void send(Ptr<Pkt>& pkt);
    ~Sender();
};

inline Sender::Sender(Ptr<UsrChan>& chn): chan(chn){}

inline void Sender::send(Ptr<Pkt> &pkt) {
    pkt->write(pktBuf);
    chan->write(pktBuf, PACKET_LEN);
}