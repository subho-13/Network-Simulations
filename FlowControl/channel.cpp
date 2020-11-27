#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>

using namespace std;

#include "signal.h"

#include "header/AdminChan.hpp"

atomic<bool> stop;

void handler(int sig) {
    stop = true;
}

int main() try {
    signal(SIGINT, handler);
    
    AdminChan adminChan;
    while(!stop) {
        printf("\b-[-]->");
        fflush(stdout);
        adminChan.taint();
        this_thread::yield();
        this_thread::sleep_for(chrono::milliseconds(200));
    } 

    cout << "\n";
} catch(Error e) {
    e.show();
}