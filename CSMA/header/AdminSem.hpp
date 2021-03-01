#pragma once
#include <semaphore.h>
#include <fcntl.h>

#include "Error.hpp"

class AdminSem {
    sem_t* sem;
    string name;
public:
    AdminSem(string name, int value);
    ~AdminSem();
};

inline AdminSem::AdminSem(string name, int value) {
    sem = sem_open(name.c_str(), O_CREAT|O_EXCL, S_IWUSR|S_IRUSR, value);
    this->name = name;

    Error error;
    error.className = "AdminSem";
    error.funcName = "Constructor";

    if(sem == SEM_FAILED) {
        error.problem = "sem_open for " + name + "failed";
        throw error;
    }
}

inline AdminSem::~AdminSem() {
    sem_close(sem);
    sem_unlink(name.c_str());
}