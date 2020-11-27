#include "../header/Semaphore.hpp"

#include <iostream>

using namespace std;

int main() {
    Semaphore mutex(1);

    while(true) {
        mutex.wait();
        // cout << "Yo\n";
        mutex.signal();
    }
}