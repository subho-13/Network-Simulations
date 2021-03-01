#pragma once
#include <semaphore.h>
#include <fcntl.h>

#include "Error.hpp"

class UsrSem {
    sem_t* sem;
public:
    UsrSem(string name);
    void wait();
    void signal();
    ~UsrSem();
};

inline UsrSem::UsrSem(string name) {
    Error error;

    error.className = "UsrSem";
    error.funcName = "Constructor";

    sem = sem_open(name.c_str(), 0);
    if(sem == SEM_FAILED) {
        error.problem = "sem_open for " + name + " failed";
        throw error;
    }
}

inline void UsrSem::wait() {
    sem_wait(sem);
}

inline void UsrSem::signal() {
    sem_post(sem);
}

inline UsrSem::~UsrSem() {
    sem_close(sem);
}