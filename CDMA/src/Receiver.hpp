#include <bits/stdint-uintn.h>
#include <memory>
#include <string>
#include <utility>

using namespace std;

#include "Channel.hpp"
#include "Encoder.hpp"
#include "Packet.hpp"

class Receiver {
    shared_ptr<Channel> channel;
    string name;
public:
    Receiver(shared_ptr<Channel> channel, const string& name);
    pair<uint64_t, bool> receive();
};

inline Receiver::Receiver(shared_ptr<Channel> channel, const string& name) {
    this->channel = channel;
    this->name = name;
}

inline pair<uint64_t, bool> Receiver::receive() {
    vector<uint8_t> tmp = channel->receive(name);
    
    if(tmp.size() == 0) {
        return make_pair(0, false);
    }

    Packet p;
    p.deserialize(tmp);
    tmp = p.unpackData();
    
    if(tmp.size() == 0) {
        return make_pair(0, false);
    }
    
    return make_pair(Encoder::decodeUint64_t(tmp), true);
}