#pragma once

#include <bits/stdint-uintn.h>
#include <cstdint>
#include <cstdlib>
#include <vector>

using namespace std;

#include "Constant.hpp"
#include "Error.hpp"

class Packet {
    uint8_t buffer[PKT_SZ];
public:
    void packData(const vector<uint8_t>& data);
    vector<uint8_t> serialize();
    void deserialize(const vector<uint8_t>& data);
    vector<uint8_t> unpackData();
};

inline void Packet::packData(const vector<uint8_t>& data) {
    if(data.size() > DATA_LEN || data.size() == 0) {
        Error e;
        e.className = "Packet";
        e.funcName = "packData";
        e.problem = "improper length";
        throw e;
    }

    buffer[0] = (uint8_t) data.size();

    for(uint8_t i = 0; i < data.size(); i++) {
         buffer[i+1] = data[i];
    }
}

inline vector<uint8_t> Packet::serialize() {
    vector<uint8_t> tmp;

    if(buffer[0] > DATA_LEN) {
        Error e;
        throw e;
    }

    tmp.resize(PKT_SZ);

    for(uint8_t i=0; i < PKT_SZ; i++) {
        tmp[i] = buffer[i];
    }

    return tmp;
}

inline void Packet::deserialize(const vector<uint8_t> &data) {
    if(data.size() > PKT_SZ || data.size() == 0) {
        Error e;
        e.className = "Packet";
        e.funcName = "deserialize";
        e.problem = "improper length";
        throw e;
    }

    for(uint8_t i = 0; i < PKT_SZ; i++) {
        buffer[i] = data[i];
    }
}

inline vector<uint8_t> Packet::unpackData() {
    vector<uint8_t> data;

    data.resize(buffer[0]);

    for(uint8_t i = 0; i < data.size(); i++) {
        data[i] = buffer[i+1];
    }

    return data;
}