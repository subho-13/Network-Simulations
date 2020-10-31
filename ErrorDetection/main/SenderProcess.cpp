#include "../header/Packet.hpp"
#include "../header/Channel.hpp"
#include "../header/Constant.hpp"

int main() try {
    Taint::init();
    CRC::init();
    
    Channel channel;
    Packet packet;

    channel.open(CHANNEL_NAME, SEMBUFFERFULL_NAME, SEMBUFFEREMPTY_NAME);
    packet.init();

    len_t bufferLen = Packet::PACKET_LEN;
    byte_t buffer[bufferLen];

    bool tainted;
    
    while(true) {
        packet.packFrame();
        packet.getFrame(buffer, bufferLen);
        channel.write(buffer, bufferLen);
        
        for(indx_t i = 0; i < TAINT_TIMES; i++) {
            packet.taintFrame();
            packet.getFrame(buffer, bufferLen);
            channel.write(buffer, bufferLen);
        }        
    }

    return 0;
} catch (Error e) {
    e.show();
    return -1;
}