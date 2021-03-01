#pragma once

#include "Constant.hpp"

class CRC {
private:
    static const crc_t poly = 0x1EDC6F41;
    crc_t table[256];

    crc_t reflect(crc_t crc) const;
public:
    CRC();
    crc_t calc(const byte_t data[], const len_t len) const;
    bool check(const byte_t data[], const len_t len) const ;
};

inline CRC::CRC() {
    crc_t rem;
    const crc_t topbit = (1 <<(sizeof(crc_t)*8 - 1));

    for(crc_t val = 0; val < 256; val++) {
        rem = val << ((sizeof(crc_t) -1)*8);

        for(byte_t bit=8; bit > 0; bit--) {
            if (rem&topbit) {
                rem = (rem<<1)^poly;
            } else {
                rem <<= 1;
            }
        }

        table[val] = rem; 
    }
}

inline crc_t CRC::reflect(crc_t crc) const {
    crc_t reflected = 0;

    for(indx_t i = 0; i < sizeof(crc_t); i++) {
        reflected <<= 8;
        reflected |= (crc&0xff);
        crc >>= 8;
    }

    return reflected;
}

inline crc_t CRC::calc(const byte_t data[], const len_t len) const {
    crc_t crc = 0;
    byte_t byte;

    for(indx_t i = 0; i < len; i++) {
        byte = data[i]^(crc>>((sizeof(crc_t)-1)*8));
        crc = table[byte]^(crc<<8);
    }

    return reflect(crc);
}

inline bool CRC::check(const byte_t data[], const len_t len) const {
    if(calc(data, len) == 0) {
        return true;
    }

    return false;
}