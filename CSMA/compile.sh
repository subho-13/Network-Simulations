#!/bin/sh

clang++ ./main/sender.cpp -pthread -lrt -std=c++17 -march=native -o sender
clang++ ./main/receiver.cpp -pthread  -lrt -std=c++17  -march=native -o receiver
clang++ ./main/channel.cpp -pthread  -lrt -std=c++17 -march=native -o channel