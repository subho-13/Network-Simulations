#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>

using namespace std;

#include "signal.h"

#include "../header/AdminChan.hpp"
#include "../header/Ptr.hpp"

atomic<bool> stop(false);
Ptr<AdminChan> gAdmin;

void handler(int sig) {
    cout << "Received signal" << sig << "\n";
    stop = true;
    gAdmin->stopOp();
    cout << "\n";
    cout.flush();
    exit(0);
}

int main() try {
    signal(SIGINT, handler);
    AdminChan adminChan;
    Ptr<AdminChan> lAdmin(&adminChan);
    gAdmin = lAdmin;
    while(!stop) {
        adminChan.taint();
        this_thread::sleep_for(chrono::milliseconds(500));
        this_thread::yield();
   }
} catch(Error e) {
    e.show();
}
