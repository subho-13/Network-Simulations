#include <atomic>
#include <cstdio>

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
        sleep(2);
    } 

} catch(Error e) {
    e.show();
}