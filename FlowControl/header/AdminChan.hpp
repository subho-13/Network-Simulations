#pragma once

#include "semaphore.h"
#include "sys/mman.h"
#include "unistd.h"
#include "fcntl.h"

#include <bits/stdint-uintn.h>
#include <iostream>
#include <random>
using namespace std;

#include "Constant.hpp"
#include "Error.hpp"
#include <cstdint>
#include <fcntl.h>
#include "Taint.hpp"

class AdminChan {
private:
    static const len_t CHAN_LEN = PACKET_LEN+2*sizeof(indx_t);
    byte_t* mmapSpace;
    byte_t* shm;

    sem_t* reader;
    sem_t* writer;
    sem_t* regReader;
    sem_t* hasWritten;
    sem_t* turnstile1;
    sem_t* turnstile2;

    int64_t fd;
    Taint tnt;
public:
    AdminChan();
    void taint();
    ~AdminChan();
};

inline AdminChan::AdminChan(): tnt() {
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

    shm = (mmapSpace + 2*sizeof(indx_t));

    reader = sem_open(MUTEX_READER, O_CREAT|O_EXCL, S_IWUSR|S_IRUSR, 1);
    if(reader == SEM_FAILED) {
        error.problem = "sem_open for reader failed";
        throw error;
    }

    writer = sem_open(MUTEX_WRITER, O_CREAT|O_EXCL, S_IWUSR|S_IRUSR, 1);
    if(writer == SEM_FAILED) {
        error.problem = "sem_open for writer failed";
        throw error;
    }

    regReader = sem_open(MUTEX_REGREADER, O_CREAT|O_EXCL, S_IWUSR|S_IRUSR, 1);
    if(regReader == SEM_FAILED) {
        error.problem = "sem_open for regReader failed";
        throw error;
    }

    hasWritten = sem_open(SEM_HASWRITTEN, O_CREAT|O_EXCL, S_IWUSR|S_IRUSR, 0);
    if(hasWritten == SEM_FAILED) {
        error.problem = "sem_open for hasWritten failed";
        throw error;
    }

    turnstile1 = sem_open(TURNSTILE1, O_CREAT|O_EXCL, S_IWUSR|S_IRUSR, 0);
    if(turnstile1 == SEM_FAILED) {
        error.problem = "sem_open for turnstile1 failed";
        throw error;
    }

    turnstile2 = sem_open(TURNSTILE2, O_CREAT|O_EXCL, S_IWUSR|S_IRUSR, 0);
    if(turnstile2 == SEM_FAILED) {
        error.problem = "sem_open for turnstile2 failed";
        throw error;
    }
}

inline void AdminChan::taint() {
    sem_wait(writer);
    tnt.taintBurst(shm, PACKET_LEN);
    sem_post(writer);
}

inline AdminChan::~AdminChan() {
    munmap(mmapSpace, CHAN_LEN);
    close(fd);
    shm_unlink(CHAN_NAME);
    sem_close(reader);
    sem_close(writer);
    sem_close(regReader);
    sem_close(hasWritten);
    sem_close(turnstile1);
    sem_close(turnstile2);
    sem_unlink(MUTEX_READER);
    sem_unlink(MUTEX_WRITER);
    sem_unlink(MUTEX_REGREADER);
    sem_unlink(SEM_HASWRITTEN);
    sem_unlink(TURNSTILE1);
    sem_unlink(TURNSTILE2);
}