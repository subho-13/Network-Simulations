#pragma once

#include "Constant.hpp"
#include "CRC.hpp"

class Pkt {
private:
    const static indx_t SRC_ADDR_INDX = 0;
    const static indx_t DST_ADDR_INDX = SRC_ADDR_INDX + sizeof(addr_t);
    const static indx_t PKT_TYPE_INDX = DST_ADDR_INDX + sizeof(addr_t);
    const static indx_t SEQ_NUM_INDX = PKT_TYPE_INDX + sizeof(type_t);
    const static indx_t DATA_LEN_INDX = SEQ_NUM_INDX + sizeof(seq_t);
    const static indx_t DATA_INDX = DATA_LEN_INDX + sizeof(len_t);
    const static indx_t CRC_INDX = PACKET_LEN - sizeof(crc_t);

    // Stores the packet
    byte_t buffer[PACKET_LEN];
public:
    const static len_t DATA_LEN = CRC_INDX - DATA_INDX;
    const static type_t TYPE_DATA = 0x01;
    const static type_t TYPE_ACK = 0x02;

    void setAddr(addr_t src, addr_t dst);
    void setSqnm(seq_t seqNm);
    void packData(byte_t data[], len_t len);
    void packAck();
    void setCRC(const CRC& crc);
    void write(byte_t buf[]) const;
    void read(byte_t buf[]);
    bool isCorrupt(const CRC& crc) const;
    void getAddr(addr_t& src, addr_t& dst) const;
    type_t getType() const;
    seq_t getSeqNum() const;
    void unpackData(byte_t data[], len_t& len) const;
};

inline void Pkt::setAddr(addr_t src, addr_t dst) {
    *(addr_t *)(buffer + SRC_ADDR_INDX) = src;
    *(addr_t *)(buffer + DST_ADDR_INDX) = dst;
}

inline void Pkt::setSqnm(seq_t seqNm) {
    *(seq_t *)(buffer + SEQ_NUM_INDX) = seqNm;
}

inline void Pkt::packData(byte_t data[], len_t len) {
    *(type_t *)(buffer + PKT_TYPE_INDX) = Pkt::TYPE_DATA;
    *(len_t *)(buffer + DATA_LEN_INDX) = len;
    for(indx_t i = 0; i < len; i++) {
        (buffer + DATA_INDX)[i] = data[i];
    }
}

inline void Pkt::packAck() {
    *(type_t *)(buffer + PKT_TYPE_INDX) = Pkt::TYPE_ACK;
    *(len_t *)(buffer + DATA_LEN_INDX) = 0;
}

inline void Pkt::setCRC(const CRC& crc) {
    crc_t crcval = crc.calc(buffer, CRC_INDX);
    *(crc_t *)(buffer + CRC_INDX) = crcval; 
}

inline void Pkt::write(byte_t buf[]) const {
    for(indx_t i = 0; i < PACKET_LEN; i++) {
        buf[i] = buffer[i];
    }
}

inline void Pkt::read(byte_t buf[]) {
    for(indx_t i = 0; i < PACKET_LEN; i++) {
        buffer[i] = buf[i];
    }
}

inline bool Pkt::isCorrupt(const CRC& crc) const {
    return !crc.check(buffer, PACKET_LEN);
}

inline void Pkt::getAddr(addr_t& src, addr_t& dst) const {
    src = *(addr_t *)(buffer + SRC_ADDR_INDX);
    dst = *(addr_t *)(buffer + DST_ADDR_INDX);
}

inline type_t Pkt::getType() const {
    return *(type_t *)(buffer + PKT_TYPE_INDX);
}

inline seq_t Pkt::getSeqNum() const {
    return *(seq_t *)(buffer + SEQ_NUM_INDX);
}

inline void Pkt::unpackData(byte_t data[], len_t& len) const {
    len = *(len_t *)(buffer + DATA_LEN_INDX);
    for(indx_t i = 0; i < len; i++) {
        data[i] = (buffer + DATA_INDX)[i];
    }
}

