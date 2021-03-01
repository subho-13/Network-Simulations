#include "../src/Encoder.hpp"
#include <bits/stdint-uintn.h>

int main() {
    for(uint64_t i = 0; i < 100; i++) {
        cout << i << "  " << Encoder::decodeUint64_t(Encoder::encodeUint64_t(i)) << "\n";
    }
}