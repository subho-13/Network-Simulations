#pragma once

#include <iostream>

using namespace std;

class Error {
public:
    string className;
    string funcName;
    string problem;

    void show() {
        printf("%s %s %s", className.c_str(), funcName.c_str(), problem.c_str());
    }
};