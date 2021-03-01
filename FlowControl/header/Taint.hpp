#pragma once
#include "Constant.hpp"

#include "random"

using namespace std;

class Taint {
     static const len_t TABLE_LEN = 255;
     byte_t table[TABLE_LEN];
     minstd_rand0 generator;
public:
    Taint() : generator(time(NULL)%2013) {
        for (indx_t i = 1; i <= TABLE_LEN; i++) {
            table[i-1] = i;
        }

        minstd_rand0 gnrtr(time(NULL));
        generator = gnrtr;
    }

     void taint(byte_t data[], len_t len) {
        indx_t i = generator()%len;
        indx_t j = generator()%TABLE_LEN;

        data[i] ^= table[j];
    }
};
