#!/bin/sh

clang++ -O3 sender.cpp -pthread -lrt -std=c++17 -Wall -Wextra -o sender
clang++ -O3 receiver.cpp -pthread  -lrt -std=c++17 -Wall -Wextra -o receiver
clang++ -O3 channel.cpp -pthread  -lrt -std=c++17 -Wall -Wextra -o channel
