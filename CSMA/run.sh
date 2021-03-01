#!/bin/bash

# CSMA one persistent

echo "senders, efficiency, throughput, avgFwdDelay" >> onep.csv
for((i = 3; i < 128; i = 2*i + 1))
do
    printf "%d, " $i >> onep.csv
    timeout -s SIGINT 20s ./channel &
    sleep 1
    
    echo "Channel Started"

    for((j = 1; j <= i; j++))
    do
        timeout -s SIGINT 16s ./sender 1 &
    done
    
    sleep 1

    echo "Spawned senders"

    timeout -s SIGINT 12s ./receiver >> onep.csv
    sleep 7
    echo "Finished One Persistent"
done

# CSMA non persistent

echo "senders, efficiency, throughput, avgFwdDelay" >> nonp.csv
for((i = 3; i < 128; i = 2*i + 1))
do
    printf "%d, " $i >> nonp.csv
    timeout -s SIGINT 20s ./channel &
    sleep 1
    
    echo "Channel Started"

    for((j = 1; j <= i; j++))
    do
        timeout -s SIGINT 16s ./sender n &
    done
    
    sleep 1

    echo "Spawned senders"

    timeout -s SIGINT 12s ./receiver >> nonp.csv
    sleep 7
    echo "Finished Non Persistent"
done

# CSMA p persistent
echo "senders, pvalue, efficiency, throughput, avgFwdDelay" >> pp.csv
for((i = 3; i < 128; i = 2*i + 1))
do
    val=$(bc <<< "scale=5; 1/$i")
    
    printf "%d, %f, " $i $val >> pp.csv
    timeout -s SIGINT 20s ./channel &
    sleep 1
    
    echo "Channel Started"

    for((j = 1; j <= i; j++))
    do
        timeout -s SIGINT 16s ./sender p $val &
    done
    
    sleep 1
    echo "Spawned senders"

    timeout -s SIGINT 12s ./receiver >> pp.csv
    sleep 7
    echo "Finished P Persistent"
        
    val=$(bc <<< "scale=5; $val/5")
    
    for k in $(seq $val 0.25 1)
    do
        printf "%d, %f, " $i $k >> pp.csv
        timeout -s SIGINT 20s ./channel &
        sleep 1
        
        echo "Channel Started"

        for((j = 1; j <= i; j++))
        do
            timeout -s SIGINT 16s ./sender p $k &
        done
        
        sleep 1
        echo "Spawned senders"

        timeout -s SIGINT 12s ./receiver >> pp.csv
        sleep 7
        echo "Finished P Persistent"
    done
done

echo "senders, throughput, avgFwdDelay" >> cdma.csv
for((i = 3; i < 128; i = 2*i + 1))
do
    printf "%d, " $i >> cdma.csv
    timeout -s SIGINT 14s ./driver $i >> cdma.csv
    echo "Finished CDMA"
done