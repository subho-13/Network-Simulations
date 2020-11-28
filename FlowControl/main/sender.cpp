#include "../header/Constant.hpp"
#include "../header/SndDataQ.hpp"
#include "../header/SndDataPktQ.hpp"
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
#include <cstdlib>
#include <signal.h>
#include <thread>

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
Ptr<SndDataPktQ> gsndDataPktQ;
Ptr<SndDataQ> gsndDataQ;

atomic<bool> stop(false);

void handle_sig(int sig) {
    cout << "Received signal" << sig << "\n";
    stop = true;

    cout << "\n +++++++++++++++ \n";
    cout.flush();
    gsndDataPktQ->showStat();
    grcvPktQ->showStat();
    cout << "\n +++++++++++++++ \n";
    cout.flush();

    exit(0);

    gusrChan->remReader();
    gsndPktQ->stopOp();
    grcvPktQ->stopOp();
    gsndDataPktQ->stopOp();
    gsndDataQ->stopOp();
}

void send_data(Ptr<SndDataQ> sndDataQ) {
    uint64_t x = 1;
    while(!stop) {
        sndDataQ->store((byte_t *)(&x), sizeof(uint64_t));
        // cout << "\n +++++++++++++++ \n";
        // cout.flush();
        // cout << x << "\n";
        // cout.flush();
        // sndDataPktQ->showStat();
        // rcvPktQ->showStat();
        // cout << "\n +++++++++++++++ \n";
        // cout.flush();
        x++;
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
    Ptr<Sender> sender(new Sender(usrChan));
    Ptr<SndPktQ> sndPktQ(new SndPktQ(sender, src, dst, crc));
    Ptr<Receiver> receiver(new Receiver(usrChan));
    Ptr<RcvPktQ> rcvPktQ(new RcvPktQ(receiver, src, dst, crc));
    Ptr<SndDataPktQ> sndDataPktQ(new SndDataPktQ(sndPktQ, rcvPktQ));
    Ptr<SndDataQ> sndDataQ(new SndDataQ(sndDataPktQ));

    gcrc = crc;
    gusrChan = usrChan;
    greceiver = receiver;
    grcvPktQ = rcvPktQ;
    gsender = sender;
    gsndPktQ = sndPktQ;
    gsndDataPktQ = sndDataPktQ;
    gsndDataQ = sndDataQ;

    signal(SIGINT, handle_sig);

    thread t1(&SndPktQ::send, &(*sndPktQ));
    thread t2(&RcvPktQ::recv, &(*rcvPktQ));
    thread t5(&SndDataPktQ::sndData, &(*sndDataPktQ));
    thread t4(&SndDataPktQ::rcvAck, &(*sndDataPktQ));
    thread t7(&SndDataQ::send, &(*sndDataQ));

    setScheduling(t1, SCHED_RR);
    setScheduling(t2, SCHED_RR);
    setScheduling(t4, SCHED_RR);
    setScheduling(t5, SCHED_RR);
    setScheduling(t7, SCHED_RR);

    t1.detach();
    t2.detach();
    t4.detach();
    t5.detach();
    t7.detach();

    thread tf(send_data, ref(sndDataQ));
    setScheduling(tf, SCHED_RR);
    tf.join();

    return 0;
} catch(Error e) {
    e.show();
}