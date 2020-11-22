#pragma once

#include "Constant.hpp"
#include "Ptr.hpp"
#include "UsrChan.hpp"
#include "Pkt.hpp"

class Receiver {
    Ptr<UsrChan> chan;
    byte_t pktBuf[PACKET_LEN];
public:
    Receiver(Ptr<UsrChan>& chn);
    void receive(Ptr<Pkt>& pkt);
    void stopOp();
};

inline Receiver::Receiver(Ptr<UsrChan>& chn) : chan(chn) {
    chan->regNewReader();
}

inline void Receiver::receive(Ptr<Pkt>& pkt) {
    chan->read(pktBuf, PACKET_LEN);
    pkt->read(pktBuf);
}

inline void Receiver::stopOp() {
    chan->remReader();
}