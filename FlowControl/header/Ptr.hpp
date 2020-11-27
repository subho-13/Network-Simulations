#pragma once

#include "Constant.hpp"
#include "cstdint"
#include "iostream"
#include <atomic>
using namespace std;

template<typename T>
class Ptr {
private:
    atomic<indx_t>* rf;
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
inline Ptr<T>::Ptr() {
    rf = NULL;
    obj = NULL;
}

template<typename T>
inline Ptr<T>::Ptr(T* o) {
    obj = o;
    rf = new atomic<indx_t>(1);
}

template<typename T>
inline Ptr<T>::Ptr(Ptr<T>& p) {
    obj = p.obj;
    rf = p.rf;

    if(rf != NULL) {
        (*rf) = (*rf)+1;
    }
}

template<typename T>
inline Ptr<T>& Ptr<T>::operator = (Ptr<T>& p) {
    if (this != &p) {
        if(rf != NULL) {
            if((*rf) == 1) {
                delete obj;
                delete rf;
            } else {
                (*rf)--;
            }
        }

        obj = p.obj;
        rf = p.rf;

        if(rf != NULL) {
            (*rf)++;
        }
    }

    return *this;
}

template<typename T>
inline T* Ptr<T>::operator -> () {
    return obj;
}

template<typename T>
inline T& Ptr<T>::operator * () {
    return *obj;
}

template <typename T>
inline Ptr<T>::~Ptr() {
    if(rf != NULL) {
        if((*rf) == 1) {
            delete obj;
            delete rf;
        } else {
            (*rf)--;
        }
    }
}