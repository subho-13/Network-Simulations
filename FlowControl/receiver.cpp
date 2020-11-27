#include "header/Constant.hpp"
#include "header/RcvDataQ.hpp"
#include "header/RcvDataPktQ.hpp"
#include "header/SndPktQ.hpp"
#include "header/Sender.hpp"
#include "header/UsrChan.hpp"
#include "header/Receiver.hpp"
#include "header/RcvPktQ.hpp"

#include <atomic>
#include <bits/stdint-uintn.h>
#include <cstdio>
#include <thread>

using namespace std;

#include "signal.h"

atomic<bool> stop(false);

Ptr<atomic<uint64_t>> numPackets(new atomic<uint64_t>(0));
void handler(int sig) {
    cout << "\n\n-----   " << *numPackets <<"  ------\n\n";
    cout.flush();
    exit(0);
}

int main() try {
    Ptr<CRC> crc (new CRC());
    Ptr<UsrChan> usrChan(new UsrChan());
    Ptr<Receiver> receiver(new Receiver(usrChan));
    Ptr<RcvPktQ> rcvPktQ(new RcvPktQ(receiver, 2, 1, crc, numPackets));
    Ptr<Sender> sender(new Sender(usrChan));
    Ptr<SndPktQ> sndPktQ(new SndPktQ(sender));
    Ptr<RcvDataPktQ> rcvDataPktQ(new RcvDataPktQ(sndPktQ, rcvPktQ, crc));
    Ptr<RcvDataQ> rcvDataQ(new RcvDataQ(rcvDataPktQ));

    thread sndPktQSend(&SndPktQ::send, &(*sndPktQ));
    thread rcvDataPktQStore(&RcvDataPktQ::store, &(*rcvDataPktQ));
    thread rcvPktQRecv(&RcvPktQ::recv, &(*rcvPktQ));
    thread rcvDataQStore(&RcvDataQ::store, &(*rcvDataQ));

    sndPktQSend.detach();
    rcvDataPktQStore.detach();
    rcvPktQRecv.detach();
    rcvDataQStore.detach();

    int x;
    signal(SIGINT, handler);
    while(!stop) {
        rcvDataQ->collect((byte_t *)&x, sizeof(int));
        cout << "\n\n-----  " << *numPackets << "   " << x <<"  ------\n\n";
        cout.flush();
    }
    
    rcvDataQ->collect((byte_t *)&x, sizeof(int));
    cout << "\n\n-----  " << *numPackets << "   " << x <<"  ------\n\n";
    cout.flush();

} catch(Error e) {
    e.show();
}