#include <bits/stdint-uintn.h>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <string>
#include <thread>
#include <signal.h>

using namespace std;

#include "../src/Sender.hpp"
#include "../src/Receiver.hpp"

atomic<bool> stop(false);
atomic<uint64_t> avgFwdDelay(0);
atomic<uint64_t> throughput(0);

chrono::high_resolution_clock::time_point start;
chrono::high_resolution_clock::time_point ending;

// Semaphore mutex(1);

void handler(int sig) {
    ending = chrono::high_resolution_clock::now();

    uint64_t value = chrono::duration_cast<chrono::milliseconds>(
            ending-start).count();

    cout << (float)(throughput*PKT_SZ*1000)/(float)value << "," << avgFwdDelay << "\n";
    exit(0);
}

void send(uint64_t initVal, string name, shared_ptr<Channel> channel) {
    Sender sender(name, channel);

    uint64_t value = initVal+15;

    //this_thread::sleep_for(chrono::milliseconds(5000));

    while(!stop) {
        value = chrono::duration_cast<chrono::nanoseconds>(
            chrono::high_resolution_clock::now().time_since_epoch()).count();
        
        sender.send(value);
        this_thread::yield();
    }
}

void receive(string name, shared_ptr<Channel> channel) {
    Receiver receiver(channel, name);

    pair<uint64_t, bool> tmp;

    uint64_t value;

    while(!stop) {
        tmp = receiver.receive();

        if(tmp.second == false) {
            continue;
        }

        value = chrono::duration_cast<chrono::nanoseconds>(
            chrono::high_resolution_clock::now().time_since_epoch()).count();

        avgFwdDelay = (avgFwdDelay+
                    (value - tmp.first))/2;
        throughput++;
        this_thread::yield();
    }
}

void broadcast(shared_ptr<Channel> channel) {
    while(!stop) {
        this_thread::sleep_for(chrono::milliseconds(500));
        channel->flush();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, handler);
    int n = atoi(argv[1]);
    shared_ptr<Channel> channel(new Channel((uint8_t)n, (uint8_t)n));

    vector<thread> senders;
    vector<thread> receivers;

    senders.resize(n);
    receivers.resize(n);
    
    start = chrono::high_resolution_clock::now();
    for(uint64_t i = 1; i <= n; i++) {
        senders[i-1] = thread(send, i, to_string(i)+"a" , channel);
        senders[i-1].detach();
    }

    for(uint64_t i = 1; i <=n; i++) {
        receivers[i-1] = thread(receive, to_string(i)+"a", channel);
        receivers[i-1].detach();
    }

    thread t(broadcast, channel);
    t.detach();

    while(true) {
        this_thread::sleep_for(chrono::seconds(1));
    }
    
}