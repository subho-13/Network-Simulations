#include "header/Constant.hpp"
#include <atomic>
#include <cstdio>

using namespace std;

#include "signal.h"

#include "header/Sender.hpp"
#include "header/UsrChan.hpp"

atomic<bool> stop(false);

void handler(int sig) {
    stop = true;
}

int main() {
    signal(SIGINT, handler);
    UsrChan usrChan;
    usrChan.regNewReader();
    int x;
    while(stop == false) {
        usrChan.read((byte_t *)&x, sizeof(int));
        printf("Receiving number %d \n", x);
        fflush(stdout);
    }
    usrChan.remReader();
}