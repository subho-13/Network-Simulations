#pragma once
#include <bits/stdint-uintn.h>
#include <cstdint>
#include <semaphore.h>

#include "Error.hpp"

class Semaphore {
    sem_t* sem;
public:
    Semaphore(int64_t val);
    void wait();
    void signal();
    void wait(uint64_t n);
    void signal(uint64_t n);
    ~Semaphore();
};

inline Semaphore::Semaphore(int64_t val) {
    sem = new sem_t;
    int64_t rval = sem_init(this->sem, 0, val);

    if (rval == -1) {
        Error error;
        error.className = "Semaphore";
        error.funcName = "Constructor";
        error.problem = "Couldn't perform sem_init";
        throw error;
    }
}

inline void Semaphore::wait() {
    int64_t rval = sem_wait(this->sem);
    
    if(rval == -1) {
        Error error;
        error.className = "Semaphore";
        error.funcName = "wait";
        error.problem = "Couldn't perform sem_wait";
        throw error;
    }
}

inline void Semaphore::signal() {
    int64_t rval = sem_post(this->sem);
    if(rval == -1) {
        Error error;
        error.className = "Semaphore";
        error.funcName = "signal";
        error.problem = "Couldn't perform sem_post";
        throw error;
    }
}

inline void Semaphore::wait(uint64_t n) {
    for(uint64_t i = 0; i < n; i++) {
        wait();
    }
}

inline void Semaphore::signal(uint64_t n) {
    for(uint64_t i = 0; i < n; i++) {
        signal();
    }
}

inline Semaphore::~Semaphore() {
    int64_t rval = sem_destroy(this->sem);
    delete this->sem;
    if(rval == -1) {
        Error error;
        error.className = "Semaphore";
        error.funcName = "Destructor";
        error.problem = "Couldn't perform sem_destroy";
        error.show();
    }
}