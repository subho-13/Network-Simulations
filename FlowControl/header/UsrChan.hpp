#pragma once

#include "semaphore.h"
#include "fcntl.h"
#include "sys/mman.h"
#include "unistd.h"

#include "Constant.hpp"
#include "Error.hpp"
#include <bits/stdint-intn.h>
#include <cstdint>

class UsrChan{
private:
    static const len_t CHAN_LEN = PACKET_LEN+2*sizeof(indx_t);
    byte_t* shm;
    indx_t* totalReader;
    indx_t* currReader;

    sem_t* reader;
    sem_t* writer;
    sem_t* regReader;
    sem_t* hasWritten;
    sem_t* turnstile1;
    sem_t* turnstile2;

    int64_t fd;
public:
    UsrChan();
    void regNewReader();
    void remReader();
    void write(byte_t data[], len_t len);
    void read(byte_t data[], len_t len);
    ~UsrChan();
};

inline void UsrChan::regNewReader() {
    sem_wait(reader);

    sem_wait(regReader);
    (*totalReader)++;
    sem_post(regReader);

    sem_post(reader);
}

inline UsrChan::UsrChan() {
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

    totalReader = (indx_t *)mmapSpace;
    currReader = (indx_t *)mmapSpace + sizeof(indx_t);
    shm = mmapSpace + 2*sizeof(indx_t);

    reader = sem_open(MUTEX_READER, 0);
    if(reader == SEM_FAILED) {
        error.problem = "sem_open for reader failed";
        throw error;
    }

    writer = sem_open(MUTEX_WRITER, 0);
    if(writer == SEM_FAILED) {
        error.problem = "sem_open for writer failed";
        throw error;
    }

    regReader = sem_open(MUTEX_REGREADER, 0);
    if(regReader == SEM_FAILED) {
        error.problem = "sem_open for regReader failed";
        throw error;
    }

    hasWritten = sem_open(SEM_HASWRITTEN, 0);
    if(hasWritten == SEM_FAILED) {
        error.problem = "sem_open for hasWritten failed";
        throw error;
    }

    turnstile1 = sem_open(TURNSTILE1, 0);
    if(turnstile1 == SEM_FAILED) {
        error.problem = "sem_open for turnstile1 failed";
        throw error;
    }

    turnstile2 = sem_open(TURNSTILE2, 0);
    if(turnstile2 == SEM_FAILED) {
        error.problem = "sem_open for turnstile2 failed";
        throw error;
    }
}

inline void UsrChan::write(byte_t data[], len_t len) {
    sem_wait(writer);
    for(indx_t i = 0; i < len; i++) {
        shm[i] = data[i];
    }
    sem_post(hasWritten);
}

inline void UsrChan::read(byte_t data[], len_t len) {
    sem_wait(reader);
    (*currReader)++;
    if (*currReader == *totalReader) {
        sem_wait(hasWritten);
        sem_wait(regReader);
        for(indx_t i = 0; i < *totalReader; i++) {
            sem_post(turnstile1);
        }
    }
    sem_post(reader);
    
    sem_wait(turnstile1);

    for(indx_t i = 0;i < len; i++) {
        data[i] = shm[i];
    }

    sem_wait(reader);
    (*currReader)--;
    if (*currReader == 0) {
        sem_post(writer);
        for(indx_t i = 0; i < *totalReader; i++) {
            sem_post(turnstile2);
        }
        sem_post(regReader);
    }
    sem_post(reader);

    sem_wait(turnstile2);
}

inline void UsrChan::remReader() {
    sem_wait(reader);

    sem_wait(regReader);
    (*totalReader)--;
    sem_post(regReader);
    
    sem_post(reader);
}

inline UsrChan::~UsrChan() {
    close(fd);
    sem_close(reader);
    sem_close(writer);
    sem_close(regReader);
    sem_close(hasWritten);
    sem_close(turnstile1);
    sem_close(turnstile2);
}