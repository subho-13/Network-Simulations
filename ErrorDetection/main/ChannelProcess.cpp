#include "../header/Channel.hpp"
#include "../header/Constant.hpp"

#include "signal.h"
#include "unistd.h"

bool stop = false;

void handle(int signal) {
    stop = true;
}

int main() {
    signal(SIGINT, handle);

    Channel channel;
    channel.init(CHANNEL_NAME, SEMBUFFERFULL_NAME, SEMBUFFEREMPTY_NAME);

    while(!stop) {
        sleep(2);
    }

    channel.close();
    return 0;
}