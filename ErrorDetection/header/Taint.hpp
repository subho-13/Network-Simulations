#include "Constant.hpp"

#include "random"

using namespace std;

class Taint {
    static const len_t bitTaintTableLen = 8;
    
    static const len_t burstTaintTableLen = 247;
    static byte_t bitTaintTable[bitTaintTableLen];
    static byte_t burstTaintTable[burstTaintTableLen];

    static minstd_rand0 generator;
public:
    static void init() {
        byte_t tmp = 1;
        for (indx_t i = 0; i < bitTaintTableLen; i++) {
            bitTaintTable[i] = tmp;
            tmp <<= 1;
        }

        bool isPresent;
        indx_t i = 0;
        byte_t val = 1;
        while (i < burstTaintTableLen) {
            isPresent = false;

            for (indx_t j = 0; j < bitTaintTableLen; j++) {
                if(bitTaintTable[j] == i) {
                    isPresent = true;
                    break;
                }
            }

            if (!isPresent) {
                burstTaintTable[i] = val;
                i++;
            }
            val++;
        }

        minstd_rand0 gnrtr(0);
        generator = gnrtr;
    }

    static void taintBit(byte_t data[], len_t len) {
        indx_t i = generator()%len;
        indx_t j = generator()%bitTaintTableLen;

        data[i] ^= bitTaintTable[j];
    }

    static void taintBurst(byte_t data[], len_t len) {
        indx_t i = generator()%len;
        indx_t j = generator()%burstTaintTableLen;

        data[i] ^= burstTaintTable[j];
    }
};