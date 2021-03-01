#pragma once
#include <bits/stdint-uintn.h>
#include <ctime>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>
#include <random>

using namespace std;

#include "UsrSem.hpp"
#include "Constant.hpp"

class UsrChan {
    int fd;
    byte_t* packetSpace;
    byte_t* numWriters;
    byte_t* realNumWriters;
    UsrSem writer;
    UsrSem haswritten;
    UsrSem hasread;
public:
    UsrChan();
    void write(byte_t data[], len_t len);
    void read(byte_t data[], len_t len);
    bool isBusy();
    ~UsrChan();
};

inline UsrChan::UsrChan() : 
writer(WRITER),
haswritten(HASWRITTEN),
hasread(HASREAD) {
    Error error;
    error.className = "UsrChan";
    error.funcName = "Constructor";
    
    fd = shm_open(CHAN_NAME, O_RDWR, 0);
    if (fd == -1) {
        error.problem = "shm_open failed";
        throw error;
    }

    int64_t rval = ftruncate(fd, CHAN_LEN);
    if (rval == -1) {
        error.problem = "ftruncate failed";
        throw error;
    }

    byte_t* mmapSpace = (byte_t *)mmap(NULL, CHAN_LEN, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(mmapSpace == MAP_FAILED) {
        error.problem = "mmap failed";
        throw error;
    }
    numWriters = mmapSpace;
    realNumWriters = (mmapSpace + 1);
    packetSpace = (mmapSpace + 2);
}

inline void UsrChan::write(byte_t data[], len_t len) {
    byte_t tmp = *numWriters;

    this_thread::yield();

    tmp = tmp + 1;
    
    *numWriters = tmp;

    writer.wait();
        (*realNumWriters)++;
    writer.signal();

    for(indx_t i = 0; i < len; i++) {
        packetSpace[i] ^= data[i];
    }

    writer.wait();
        (*realNumWriters)--;
        if(*realNumWriters == 0) {
            haswritten.signal();            
            hasread.wait();

            for(indx_t i = 0; i < len; i++) {
                packetSpace[i] = 0;
            }
            
            *numWriters = 0;
        }
    writer.signal();
}

inline void UsrChan::read(byte_t data[], len_t len) {
    haswritten.wait();

    for(indx_t i = 0; i < len; i++) {
        data[i] = packetSpace[i];
    }

    hasread.signal();
}

inline bool UsrChan::isBusy() {
    byte_t tmp = *numWriters;

    this_thread::yield();

    if(tmp == 0) {
        return false;
    }

    return true;
}

inline UsrChan::~UsrChan() {
    close(fd);
}
