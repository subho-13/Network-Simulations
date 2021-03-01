#!/bin/bash

./compile.sh
for i in {1..5}
do
    ./testScript.sh
done
