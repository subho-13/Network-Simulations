#include "../src/Walsh.hpp"
#include "../src/Packet.hpp"
#include "../src/Encoder.hpp"

#include "iostream"
#include <bits/stdint-intn.h>
#include <cstdint>

using namespace std;

int main() {
    Walsh walsh(10);
    Packet p;
    char* c = NULL;
    char newchar;
    uint8_t len;
    for(uint8_t i = 0; i < 10; i++) {
        newchar = 'a' + i;
        p.packData(Encoder::encodeChar(newchar));
        auto tmp1 = p.serialize();
        auto tmp2 = walsh.encodeData(i, tmp1);
        p.deserialize(walsh.decodeData(i, tmp2));
        auto tmp3 = p.unpackData();
        cout << Encoder::decodeChar(tmp3) << "\n";
    }
}