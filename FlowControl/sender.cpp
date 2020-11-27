#include "header/Constant.hpp"
#include "header/SndDataQ.hpp"
#include "header/SndDataPktQ.hpp"
#include "header/SndPktQ.hpp"
#include "header/Sender.hpp"
#include "header/UsrChan.hpp"
#include "header/Receiver.hpp"
#include "header/RcvPktQ.hpp"

#include <atomic>
#include <bits/stdint-uintn.h>
#include <chrono>
#include <cstdio>
#include <thread>

using namespace std;

#include "signal.h"

atomic<bool> stop(false);

Ptr<atomic<uint64_t>> numPackets(new atomic<uint64_t>(0));
Ptr<atomic<uint64_t>> avgRtt(new atomic<uint64_t>(200000));

void handler(int sig) {
    cout << "\n\n-----  " << *avgRtt << "   " << *numPackets <<"  ------\n\n";
    cout.flush();
    exit(0);
}

int main() try {
    Ptr<CRC> crc (new CRC());
    Ptr<UsrChan> usrChan(new UsrChan());
    Ptr<Sender> sender(new Sender(usrChan));    
    Ptr<SndPktQ> sndPktQ(new SndPktQ(sender));
    Ptr<Receiver> receiver(new Receiver(usrChan));
    Ptr<RcvPktQ> rcvPktQ(new RcvPktQ(receiver, 1, 2, crc, numPackets));
    Ptr<SndDataPktQ> sndDataPktQ(new SndDataPktQ(sndPktQ, rcvPktQ, avgRtt));
    Ptr<SndDataQ> sndDataQ(new SndDataQ(sndDataPktQ, crc, 1, 2));
    
    thread sndPktQSend(&SndPktQ::send, &(*sndPktQ));
    thread rcvPktQRecv(&RcvPktQ::recv, &(*rcvPktQ));
    thread sndDataPktQSndData(&SndDataPktQ::sndData, &(*sndDataPktQ));
    thread sndDataPktQRcvAck(&SndDataPktQ::rcvAck, &(*sndDataPktQ));    
    thread sndDataQSend(&SndDataQ::send, &(*sndDataQ));

    sndPktQSend.detach();
    rcvPktQRecv.detach();
    sndDataPktQSndData.detach();
    sndDataPktQRcvAck.detach();
    sndDataQSend.detach();

    int x = 0;
    signal(SIGINT, handler);
    while(true) {
        cout << "\n\n-----  " << *avgRtt << "   " << x <<"  ------\n\n";
        cout.flush();
        sndDataQ->store((byte_t*)(&x), sizeof(int));
        x++;
    }
    cout << "\n\n-----  " << *avgRtt << "   " << x <<"  ------\n\n";
    cout.flush();
} catch(Error e) {
    e.show();
}