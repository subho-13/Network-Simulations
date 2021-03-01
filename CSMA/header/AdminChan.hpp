#pragma once

#include <sys/mman.h>
#include <unistd.h>

#include "AdminSem.hpp"
#include "Constant.hpp"

class AdminChan {
    int fd;
    byte_t* mmapSpace;

    AdminSem haswritten;
    AdminSem hasread;
    AdminSem writer;
public:
    AdminChan();
    ~AdminChan();
};

inline AdminChan::AdminChan() :
haswritten(HASWRITTEN, 0), 
hasread(HASREAD, 0),
writer(WRITER, 1) {
    Error error;
    error.className = "AdminChan";
    error.funcName = "Constructor";

    fd = shm_open(CHAN_NAME,O_CREAT|O_EXCL|O_RDWR, S_IRUSR|S_IWUSR);
    if (fd == -1) {
        error.problem = "shm_open failed";
        throw error;
    }

    int64_t rval = ftruncate(fd, CHAN_LEN);
    if (rval == -1) {
        error.problem = "ftruncate failed";
        throw error;
    }

    mmapSpace = (byte_t *)mmap(NULL, CHAN_LEN, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(mmapSpace == MAP_FAILED) {
        error.problem = "mmap failed";
        throw error;
    }

    for (indx_t i =0; i < CHAN_LEN; i++) {
        mmapSpace[i] = 0;
    }
}

inline AdminChan::~AdminChan() {
    munmap(mmapSpace, CHAN_LEN);
    close(fd);
    shm_unlink(CHAN_NAME);
}