#include "../header/Constant.hpp"
#include "../header/RcvDataQ.hpp"
#include "../header/RcvDataPktQ.hpp"
#include "../header/SndPktQ.hpp"
#include "../header/Sender.hpp"
#include "../header/UsrChan.hpp"
#include "../header/Receiver.hpp"
#include "../header/RcvPktQ.hpp"
#include "../header/Error.hpp"
#include "../header/SndDataQ.hpp"
#include "../header/SndDataPktQ.hpp"

#include <bits/stdint-uintn.h>
#include <cstdint>
#include <signal.h>

void setScheduling(thread &th, int policy) {
    sched_param sch_params;
    sch_params.sched_priority = 0;
    pthread_setschedparam(th.native_handle(), policy, &sch_params);
}

addr_t map(char* addr) {
    addr_t mappedAddr = 2;

    while(*addr != '\0') {
        mappedAddr *= (*addr);
        mappedAddr <<= 1;
        addr++;
    }

    return mappedAddr;
}

Ptr<CRC> gcrc;
Ptr<SndPktQ> gsndPktQ;
Ptr<Sender> gsender;
Ptr<UsrChan> gusrChan;
Ptr<Receiver> greceiver;
Ptr<RcvPktQ> grcvPktQ;
Ptr<RcvDataPktQ> grcvDataPktQ;
Ptr<RcvDataQ> grcvDataQ;

atomic<bool> stop(false);

void handle_sig(int sig) {
    cout << "Received signal" << sig << "\n";
    stop = true;

    cout << "\n +++++++++++++++ \n";
    cout.flush();
    grcvPktQ->showStat();
    cout << "\n +++++++++++++++ \n";
    cout.flush();

    exit(0);

    gusrChan->remReader();
    gsndPktQ->stopOp();
    grcvPktQ->stopOp();
    grcvDataPktQ->stopOp();
    grcvDataQ->stopOp();
}

void recv_data(Ptr<RcvDataQ> rcvDataQ) {
    uint64_t x;
    while(!stop) {
        rcvDataQ->collect((byte_t *)(&x), sizeof(uint64_t));
        // cout << "\n +++++++++++++++ \n";
        // cout << x << "\n";
        // cout.flush();
        // rcvPktQ->showStat();
        // cout << "\n +++++++++++++++ \n";
        // cout.flush();
    }
}

int main(int argc, char** argv) try {
    if(argc != 3) {
        Error e;
        e.problem = "Arguments not given properly";
    }

    addr_t src, dst;
    src = map(argv[1]);
    dst = map(argv[2]);

    Ptr<CRC> crc (new CRC());
    Ptr<UsrChan> usrChan(new UsrChan());
    Ptr<Receiver> receiver(new Receiver(usrChan));
    Ptr<RcvPktQ> rcvPktQ(new RcvPktQ(receiver, src, dst, crc));
    Ptr<Sender> sender(new Sender(usrChan));
    Ptr<SndPktQ> sndPktQ(new SndPktQ(sender, src, dst, crc));
    Ptr<RcvDataPktQ> rcvDataPktQ(new RcvDataPktQ(sndPktQ, rcvPktQ));
    Ptr<RcvDataQ> rcvDataQ(new RcvDataQ(rcvDataPktQ));

    gcrc = crc;
    gusrChan = usrChan;
    greceiver = receiver;
    grcvPktQ = rcvPktQ;
    gsender = sender;
    gsndPktQ = sndPktQ;
    grcvDataPktQ = rcvDataPktQ;
    grcvDataQ = rcvDataQ;

    signal(SIGINT, handle_sig);

    thread t1(&SndPktQ::send, &(*sndPktQ));
    thread t5(&RcvDataPktQ::store, &(*rcvDataPktQ));
    thread t2(&RcvPktQ::recv, &(*rcvPktQ));
    thread t7(&RcvDataQ::store, &(*rcvDataQ));

    setScheduling(t1, SCHED_RR);
    setScheduling(t2, SCHED_RR);
    setScheduling(t5, SCHED_RR);
    setScheduling(t7, SCHED_RR);

    t1.detach();
    t2.detach();
    t5.detach();
    t7.detach();

    thread tf(recv_data, ref(rcvDataQ));
    setScheduling(tf, SCHED_RR);
    tf.join();

    return 0;
} catch(Error e) {
    e.show();
}