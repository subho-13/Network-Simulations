#pragma once

#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <cstdint>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <atomic>
#include <thread>
using namespace std;

#include "Constant.hpp"
#include "Walsh.hpp"
#include "Semaphore.hpp"

class Channel {
    uint8_t numSenders;
    uint8_t numReaders;
    int currReaders;

    Semaphore hasWritten;
    Semaphore hasRead;
    Semaphore mWriter;
    Semaphore mRegs;
    Semaphore mReader;
    Semaphore turnstile1;
    Semaphore turnstile2;
    
    Walsh walsh;

    // Store Data being sent
    vector<vector<int8_t>> storage;

    // Table to find if a sender has sent data
    map<uint8_t, bool> hasSent;

    // map Sender Name to Walsh Id
    map<string, uint8_t> senderId; 

    // Broadcast packet w.r.t some policy
    void broadcast();

    // clear storage space
    void clear();

    // add to storage
    void add(const vector<vector<int8_t>>& data);
public:
    Channel(uint8_t numReaders, uint8_t numSenders);

    // Register Sender to assign an id
    void reg(const string& name);

    // Send Data
    void send(const string& name, const vector<uint8_t>& data);

    // Flush all data that's written
    void flush();

    // Receive Data from a sender
    vector<uint8_t> receive(const string& name);
};

inline Channel::Channel(uint8_t numReaders, uint8_t numSenders):
walsh(numSenders), hasWritten(0), hasRead(0), mWriter(1), mRegs(1),
mReader(1), turnstile1(0), turnstile2(0){
    this->numReaders = numReaders;
    this->numSenders = numSenders;
    
    storage.resize(PKT_SZ*8);

    for(int i = 0; i < storage.size(); i++) {
        storage[i].resize(walsh.getSize());
    }

    clear();
}

inline void Channel::clear() {
    for(int i = 0; i < storage.size(); i++) {
        for(int j = 0; j < storage[i].size(); j++) {
            storage[i][j] = 0;
        }
    }
}

inline void Channel::add(const vector<vector<int8_t>>& data) {
    for(int i = 0; i < data.size(); i++) {
        for(int j = 0; j < data[i].size(); j++) {
            storage[i][j] += data[i][j];
        }
    }
}

inline void Channel::reg(const string& name) {
    mRegs.wait();
    if(senderId.find(name) == senderId.end()) {
        senderId[name] = walsh.getId();
    }
    mRegs.signal();
}

inline void Channel::broadcast() {
    hasWritten.signal();
    hasRead.wait();
    clear();
    hasSent.clear();
}

inline void Channel::send(const string& name, const vector<uint8_t> &data) {
    uint8_t id = senderId[name];

    vector<vector<int8_t>> encoded = walsh.encodeData(id, data);

    mWriter.wait();

    if(hasSent.find(id) != hasSent.end()) {
        broadcast();
    } 

    hasSent[id] = true;

    add(encoded);

    mWriter.signal();
}

inline void Channel::flush() {
    mWriter.wait();
    broadcast();
    mWriter.signal();
}

inline vector<uint8_t> Channel::receive(const string& name) {
    vector<uint8_t> data;
    
    mReader.wait();
    currReaders++;
    if(currReaders == numReaders) {
        hasWritten.wait();
        turnstile1.signal(numReaders);
    }
    mReader.signal();

    turnstile1.wait();

    data = walsh.decodeData(senderId[name], storage);

    mReader.wait();
    currReaders--;
    if(currReaders == 0) {
        hasRead.signal();
        turnstile2.signal(numReaders);
    }
    mReader.signal();

    turnstile2.wait();
    
    return data;
}