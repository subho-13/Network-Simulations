#pragma once

#include <bits/stdint-uintn.h>
#include <cstdint>
#include <vector>

using namespace std;

#include "Error.hpp"

class Encoder {
public:
    static vector<uint8_t> encodeChar(char c);
    static char decodeChar(vector<uint8_t> data);
    static vector<uint8_t> encodeUint64_t(uint64_t value);
    static uint64_t decodeUint64_t(vector<uint8_t> data);
};

inline vector<uint8_t> Encoder::encodeChar(char c) {
    vector<uint8_t> tmp;
    tmp.push_back((uint8_t) c);
    return tmp;
}

inline char Encoder::decodeChar(vector<uint8_t> data) {
    if(data.size() != 1) {
        Error e;
        e.className = "Encoder";
        e.funcName = "decodeChar";
        e.problem = "improper length";
        throw e;
    }
    
    char c;
    c = (char) data[0];
    return c;
}

inline vector<uint8_t> Encoder::encodeUint64_t(uint64_t value) {
    uint64_t mask = 0xff;
    uint8_t tmp;

    vector<uint8_t> encoded;

    for(int bytenum = 1; bytenum <= 8; bytenum++) {
        tmp = value&mask;
        encoded.push_back(tmp);
        value >>= 8;
    }

    return encoded;
}

inline uint64_t Encoder::decodeUint64_t(vector<uint8_t> data) {
    if(data.size() != 8) {
        Error e;
        e.className = "Encoder";
        e.funcName = "decodeUint64_t";
        e.problem = "improper length";
        throw e;
    }
    
    uint64_t value = 0;

    for(int bytenum = 8; bytenum >= 1; bytenum--) {
        value <<= 8;
        value |= data[bytenum-1];
    }

    return value;
}