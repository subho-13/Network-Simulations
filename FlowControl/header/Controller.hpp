#pragma once

#include "Constant.hpp"
#include "SndDataQ.hpp"
#include "SndDataPktQ.hpp"
#include "SndPktQ.hpp"
#include "Sender.hpp"
#include "UsrChan.hpp"
#include "Receiver.hpp"
#include "RcvPktQ.hpp"
#include "RcvDataPktQ.hpp"
#include "RcvDataQ.hpp"
#include "Ptr.hpp"

class Controller {
    Ptr<CRC> crc;
    Ptr<SndDataQ> sndDataQ;
    Ptr<SndDataPktQ> sndDataPktQ;
    Ptr<SndPktQ> sndPktQ;
    Ptr<Sender> sender;
    Ptr<UsrChan> usrChan;
    Ptr<Receiver> receiver;
    Ptr<RcvPktQ> rcvPktQ;
    Ptr<RcvDataPktQ> rcvDataPktQ;
    Ptr<RcvDataQ> rcvDataQ;
public:
    Controller(addr_t srcAddr, addr_t dstAddr);
    void send(byte_t data[], len_t len);
    void recv(byte_t data[], len_t len);
    void stopOp();
};

inline Controller::Controller(addr_t srcAddr, addr_t dstAddr):
crc(new CRC()),
usrChan(new UsrChan()), 
sender(new Sender(usrChan)),
receiver(new Receiver(usrChan)),
sndPktQ(new SndPktQ(sender)),
rcvPktQ(new RcvPktQ(receiver, srcAddr, dstAddr, crc)),
sndDataPktQ(new SndDataPktQ(sndPktQ, rcvPktQ)),
rcvDataPktQ(new RcvDataPktQ(sndPktQ, rcvPktQ, crc)),
sndDataQ(new SndDataQ(sndDataPktQ, crc, srcAddr, dstAddr)),
rcvDataQ(new RcvDataQ(rcvDataPktQ))
{}

inline void Controller::send(byte_t data[], len_t len) {
    sndDataQ->store(data, len);
}

inline void Controller::recv(byte_t data[], len_t len) {
    rcvDataQ->collect(data, len);
}

inline void Controller::stopOp() {
    usrChan->remReader();
    sndPktQ->stopOp();
    rcvPktQ->stopOp();
    sndDataPktQ->stopOp();
    rcvDataPktQ->stopOp();
    sndDataQ->stopOp();
    rcvDataQ->stopOp();
}