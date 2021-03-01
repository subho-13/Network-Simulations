#!/bin/bash
timeout -s SIGINT 40s ./channel &
sleep 4
timeout -s SIGINT 40s ./receiver 2 1 &
timeout -s SIGINT 40s ./sender 1 2 &>> sender.log
