#include "../header/Ptr.hpp"

#include "unistd.h"
#include <iostream>

using namespace std;

int main() {
    Ptr<int> p1(new int(10));
    int x = 99999999;
    while(x--) {
        Ptr<int> p2 = p1;
        Ptr<int> nothing;
    }
}