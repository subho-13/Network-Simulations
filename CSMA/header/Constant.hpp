#pragma once

#include "cstdint"
#include "iostream"
#include <cstdint>

using namespace std;

typedef uint8_t byte_t;
typedef uint16_t len_t;
typedef len_t indx_t;
typedef uint8_t addr_t;
typedef uint8_t type_t;
typedef uint16_t seq_t;
typedef uint32_t crc_t;

const len_t PACKET_LEN = 64;
const seq_t MAXSEQNO   = UINT16_MAX;

const len_t CHAN_LEN = 64 + 2*1;

const char HASWRITTEN[] = "haswritten";
const char CHAN_NAME[] = "csmachan";
const char WRITER[] = "writer";
const char HASREAD[] = "hasread";

const int SENDTIMES = 500000;
