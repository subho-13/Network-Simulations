#pragma once

#include <iostream>

using namespace std;

class Error {
public:
    string className;
    string funcName;
    string problem;

    void show() {
        printf("\n%s : %s \n%s \n", 
        className.c_str(), funcName.c_str(), problem.c_str());
    }
};