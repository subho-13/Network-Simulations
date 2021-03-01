#pragma once
#include <chrono>
#include <memory>

using namespace std;

#include "Packet.hpp"
#include "CRC.hpp"

class PktGntr {
    chrono::high_resolution_clock::time_point now;
    long val;
    CRC crc;
public:
    unique_ptr<Pkt> generate();
};

inline unique_ptr<Pkt> PktGntr::generate() {
    unique_ptr<Pkt> pktPtr(new Pkt());
    now = chrono::high_resolution_clock::now();
    val = chrono::duration_cast<chrono::nanoseconds>
            (now.time_since_epoch()).count();
    
    pktPtr->packData((byte_t*)&val, sizeof(long));
    pktPtr->setCRC(crc);

    return pktPtr;
}