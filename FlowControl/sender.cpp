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
    UsrChan usrChan;
    int x = 0;
    while(stop == false) {
        printf("Sending number %d \n", x);
        usrChan.write((byte_t *)&x, sizeof(int));
        fflush(stdout);
        sleep(2);
        x++;
    }
}