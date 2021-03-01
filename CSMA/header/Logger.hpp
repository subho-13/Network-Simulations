#pragma once

#include "Constant.hpp"
#include <bits/stdint-uintn.h>
#include <cstdint>
#include <chrono>
#include <memory>
#include <cstdio>

using namespace std;

#include "Packet.hpp"
#include "CRC.hpp"

class Logger {
    uint64_t nmPkt;
    uint64_t nmUCPkt;
    uint64_t avgFwdDly;

    chrono::high_resolution_clock::time_point start; 
    chrono::high_resolution_clock::time_point end;
    chrono::high_resolution_clock::time_point now;
    CRC crc;
    
    long val1;
    long val2;
    len_t len;
public:
    Logger();
    void log(unique_ptr<Pkt> pkt);
    ~Logger();
};

inline Logger::Logger() {
    nmPkt = 0;
    nmUCPkt = 0;
    avgFwdDly = 0;

    start = chrono::high_resolution_clock::now();
}

inline void Logger::log(unique_ptr<Pkt> pkt) {
    nmPkt++;

    if (!pkt->isCorrupt(crc)) {
        nmUCPkt++;
    } else {
        return;
    }

    now = chrono::high_resolution_clock::now();
    val2 = chrono::duration_cast<chrono::nanoseconds>
            (now.time_since_epoch()).count();

    pkt->unpackData((byte_t *)&val1, len);

    if(val2 > val1) {
        avgFwdDly += (val2 - val1);
        avgFwdDly /= 2;
    }    
}

inline Logger::~Logger() {
    end = chrono::high_resolution_clock::now();

    long timeDiff = chrono::duration_cast<chrono::milliseconds>
            (end - start).count();

    float efficiency = (float)nmUCPkt/(float)nmPkt;
    float throughput = (float)(nmUCPkt*PACKET_LEN*1000)/(float)timeDiff;

    printf("%f, %f, %ld\n", efficiency, throughput, avgFwdDly);
}
