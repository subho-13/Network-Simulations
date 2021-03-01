#pragma once
#include <bits/stdint-uintn.h>
#include <memory>
#include <string>

using namespace std;

#include "Channel.hpp"
#include "Encoder.hpp"
#include "Packet.hpp"

class Sender {
    shared_ptr<Channel> channel;
    string name;
public:
    Sender(const string& name, shared_ptr<Channel> channel);
    void send(uint64_t value);
};

inline Sender::Sender(const string& name, shared_ptr<Channel> channel) {
    this->name = name;
    this->channel = channel;
    this->channel->reg(name);
}

inline void Sender::send(uint64_t value) {
    Packet p;
    p.packData(Encoder::encodeUint64_t(value));
    this->channel->send(name, p.serialize());
}