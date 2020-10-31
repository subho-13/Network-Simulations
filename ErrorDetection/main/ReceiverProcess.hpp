#include "../header/Packet.hpp"
#include "../header/Channel.hpp"
#include "../header/Constant.hpp"
#include "../header/Log.hpp"`11

int main() try {
    Taint::init();
    CRC::init();

    Channel channel;
    channel.open(CHANNEL_NAME, SEMBUFFERFULL_NAME, SEMBUFFEREMPTY_NAME);

    Log log;
    Packet untaintedPacket, taintedPacket;

    bool tainted;
    len_t bufferLen = Packet::PACKET_LEN;
    byte_t taintedBuffer[bufferLen];
    byte_t untaintedBuffer[bufferLen];
    byte_t errorType;

    while(true) {
        channel.read(untaintedBuffer, bufferLen);
        untaintedPacket.unpackFrame(untaintedBuffer, bufferLen);
        errorType = packetCompare(untaintedPacket, untaintedPacket);
        log.write(untaintedBuffer, untaintedBuffer, bufferLen, errorType);

        for(indx_t i = 0; i < TAINT_TIMES; i++) {
            channel.read(taintedBuffer, bufferLen);
            taintedPacket.unpackFrame(taintedBuffer, bufferLen);
            errorType = packetCompare(untaintedPacket, taintedPacket);
            log.write(untaintedBuffer, taintedBuffer, bufferLen, errorType);
        }
    }
} catch (Error e) {
    e.show();
}