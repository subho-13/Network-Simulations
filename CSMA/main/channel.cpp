#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include <memory>

using namespace std;

#include "signal.h"

#include "../header/AdminChan.hpp"
#include "../header/Semaphore.hpp"

Semaphore mutex(0);

void handler(int sig) {
    mutex.signal();
    return;
}

int main() try {
    signal(SIGINT, handler);
    AdminChan channel;
    mutex.wait();
    channel.~AdminChan();
} catch(Error e) {
    e.show();
}