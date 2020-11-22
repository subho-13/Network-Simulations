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
typedef uint32_t seq_t;
typedef uint32_t crc_t;

const len_t PACKET_LEN = 128;
const len_t WINDOW_LEN = 32;
const seq_t MAXSEQNO   = UINT32_MAX;

const char CHAN_NAME[] = "flowchan";
const char MUTEX_READER[] = "reader";
const char TURNSTILE1[] = "turnstile1";
const char TURNSTILE2[] = "turnstile2";
const char SEM_HASWRITTEN[] = "haswritten";
const char MUTEX_WRITER[] = "writer";
const char MUTEX_REGREADER[] = "regreader";