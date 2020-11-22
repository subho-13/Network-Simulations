#pragma once
#include "semaphore.h"

#include "Error.hpp"

class Semaphore {
    sem_t sem;
public:
    Semaphore(int64_t val);
    void wait();
    void signal();
    ~Semaphore();
};

inline Semaphore::Semaphore(int64_t val) {
    int64_t rval = sem_init(&sem, 0, val);

    if (rval == 0) {
        Error error;
        error.className = "Semaphore";
        error.funcName = "Constructor";
        error.problem = "Couldn't perform sem_init";
        throw error;
    }
}

inline void Semaphore::wait() {
    int64_t rval = sem_wait(&sem);
    if(rval == 0) {
        Error error;
        error.className = "Semaphore";
        error.funcName = "wait";
        error.problem = "Couldn't perform sem_wait";
        throw error;
    }
}

inline void Semaphore::signal() {
    int64_t rval = sem_post(&sem);
    if(rval == 0) {
        Error error;
        error.className = "Semaphore";
        error.funcName = "signal";
        error.problem = "Couldn't perform sem_post";
        throw error;
    }
}

inline Semaphore::~Semaphore() {
    int64_t rval = sem_destroy(&sem);
    if(rval == 0) {
        Error error;
        error.className = "Semaphore";
        error.funcName = "Destructor";
        error.problem = "Couldn't perform sem_destroy";
        error.show();
    }
}