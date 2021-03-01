#!/bin/sh

clang++ -g3 sender.cpp -pthread -lrt -std=c++17 -Wall -Wextra -o ../sender
clang++ -g3 receiver.cpp -pthread  -lrt -std=c++17 -Wall -Wextra -o ../receiver
clang++ -g3 channel.cpp -pthread  -lrt -std=c++17 -Wall -Wextra -o ../channel