#!/bin/bash
sudo nice -n -10 timeout -s SIGINT 4m ./channel &
sleep 4
sudo nice -n -10 timeout -s SIGINT 4m ./receiver 2 1 &
sudo nice -n -10 timeout -s SIGINT 4m ./sender 1 2 &>> sender.log
