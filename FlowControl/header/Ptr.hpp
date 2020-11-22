#pragma once

#include "cstdint"
#include "semaphore.h"
#include "iostream"

using namespace std;

/**
 * Ptr: Smart Pointer
*/

class RefCnt {
    uint16_t cnt;
    sem_t mutex;
public:
    RefCnt();
    void inc();
    uint16_t get();
    void dec();
    ~RefCnt();
};

inline RefCnt::RefCnt() {
    cnt = 1;
    sem_init(&mutex, 0, 1);
}

inline void RefCnt::inc() {
    sem_wait(&mutex);
    cnt++;
    sem_post(&mutex);
}

inline void RefCnt::dec() {
    sem_wait(&mutex);
    cnt--;
    sem_post(&mutex);
}

inline uint16_t RefCnt::get() {
    return cnt;
}

inline RefCnt::~RefCnt() {
    sem_destroy(&mutex);
}

template<typename T>
class Ptr {
private:
    RefCnt* rf;
    T* obj;
public:
    Ptr();
    Ptr(T* object);
    Ptr(Ptr& p);
    Ptr& operator = (Ptr& p);
    T* operator -> ();
    T& operator * ();
    ~Ptr();
};

template<typename T>
Ptr<T>::Ptr() {
    rf = NULL;
    obj = NULL;
}

template<typename T>
Ptr<T>::Ptr(T* obj) {
    obj = obj;
    rf = new RefCnt();
}

template<typename T>
Ptr<T>::Ptr(Ptr<T>& p) {
    obj = p.obj;
    rf = p.rf;
    if(rf != NULL) {
        rf->inc();
    }
}

template<typename T>
Ptr<T>& Ptr<T>::operator = (Ptr<T>& p) {
    if (this != &p) {
        if(rf != NULL) {
            if(rf->get() == 1) {
                delete obj;
                delete rf;
            } else {
                rf->dec();
            }
        }

        obj = p.obj;
        rf = p.rf;

        if(rf != NULL) {
            rf->inc();
        }
    }

    return *this;
}

template<typename T>
T* Ptr<T>::operator -> () {
    return obj;
}

template<typename T>
T& Ptr<T>::operator * () {
    return *obj;
}

template <typename T>
Ptr<T>::~Ptr() {
    if(rf != NULL) {
        if(rf->get() == 1) {
            delete obj;
            delete rf;
        } else {
            rf->dec();
        }
    }
}