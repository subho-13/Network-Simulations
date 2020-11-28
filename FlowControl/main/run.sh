#!/bin/bash

./compile.sh
for i in {1..3}
do
    sleep 10
    ./testScript.sh
done
