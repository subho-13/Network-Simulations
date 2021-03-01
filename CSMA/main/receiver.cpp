#include <signal.h>
#include <atomic>

#include "../header/Receiver.hpp"
#include "../header/Logger.hpp"

atomic<bool> stop(false);

Logger logger;

void handler(int sig) {
    stop = true;
}

int main() {
    signal(SIGINT, handler);
    Receiver reciever;

    while(!stop) {
        logger.log(reciever.receive());
    }
}