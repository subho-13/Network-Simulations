#include "../header/Semaphore.hpp"
#include "../header/Constant.hpp"

#include <bits/stdint-uintn.h>
#include <chrono>
#include <iostream>
#include <vector>
#include <thread>

using namespace std;

int main() {
    chrono::high_resolution_clock::time_point start;
    chrono::high_resolution_clock::time_point end;

    vector<uint8_t> arr1;
    vector<uint8_t> arr2;

    arr1.resize(PACKET_LEN);
    arr2.resize(PACKET_LEN);

    uint64_t avgDelay;

    Semaphore mutex(1);

    for(int i = 0; i < 1000000; i++) {
        start = chrono::high_resolution_clock::now();
        mutex.wait();
        mutex.signal();
        this_thread::yield();
        for(int i = 0; i < arr1.size(); i++) {
            arr1[i] = arr2[i];
        }
        mutex.wait();
        mutex.signal();
        this_thread::yield();
        end = chrono::high_resolution_clock::now();

        avgDelay = (avgDelay + chrono::duration_cast<chrono::nanoseconds>(end - start).count())/2;
    }

    cout << avgDelay << "\n";
}