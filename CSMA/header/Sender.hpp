#pragma once

#include <bits/stdint-uintn.h>
#include <cstdint>
#include <memory>
#include <random>
#include <time.h>
#include <thread>
#include <chrono>

using namespace std;


#include "UsrChan.hpp"
#include "Packet.hpp"
#include "Constant.hpp"

class Sender {
    UsrChan channel;
    
    minstd_rand rndBkOffTm;
    minstd_rand rndNm;

    uint64_t mxNm;
    uint64_t p;
    uint64_t mxBkOffTm;

    uint64_t timeSlot;

    byte_t buffer[PACKET_LEN];
public:
    Sender(float p);
    void send1P(unique_ptr<Pkt> pkt);
    void sendNP(unique_ptr<Pkt> pkt);
    void sendPP(unique_ptr<Pkt> pkt);
};

inline Sender::Sender(float f) :
rndBkOffTm(random_device{}()),
rndNm(random_device{}()) {
    mxNm = 1000000;
    this->p = (uint64_t)(f*mxNm)%mxNm;
    mxBkOffTm = 30;
    timeSlot = 15;
}

inline void Sender::send1P(unique_ptr<Pkt> pkt) {
    pkt->write(buffer);

    while(channel.isBusy()) {
        this_thread::sleep_for(chrono::microseconds(10));
    }

    channel.write(buffer, PACKET_LEN);
}

inline void Sender::sendNP(unique_ptr<Pkt> pkt) {
    pkt->write(buffer);

    while(channel.isBusy()) {
        this_thread::sleep_for(chrono::microseconds(rndBkOffTm()%mxBkOffTm + 10));
    }

    channel.write(buffer, PACKET_LEN);
}

inline void Sender::sendPP(unique_ptr<Pkt> pkt) {
    pkt->write(buffer);

    uint64_t tmp;

    while(true) {
        tmp = rndNm()%mxNm;
        while(channel.isBusy()) {
            this_thread::sleep_for(chrono::microseconds(10));
        }

        if(tmp < p) {
            channel.write(buffer, PACKET_LEN);
            break;
        }

        this_thread::sleep_for(chrono::microseconds(timeSlot));

        if(channel.isBusy()) {
            this_thread::sleep_for(chrono::microseconds(rndBkOffTm()%mxBkOffTm + 10));
        }
    }
}
