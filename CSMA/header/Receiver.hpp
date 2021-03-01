#pragma once
#include "Constant.hpp"
#include <memory>

using namespace std;

#include "UsrChan.hpp"
#include "Packet.hpp"

class Receiver {
    UsrChan channel;
    byte_t buffer[PACKET_LEN];
public:
    unique_ptr<Pkt> receive();
};

inline unique_ptr<Pkt> Receiver::receive() {
    unique_ptr<Pkt> ptrPkt(new Pkt());
    channel.read(buffer, PACKET_LEN);
    ptrPkt->read(buffer);
    return ptrPkt;
}