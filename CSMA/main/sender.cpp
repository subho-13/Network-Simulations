#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "../header/Sender.hpp"
#include "../header/PktGntr.hpp"


// ./sender mode [val of p]
int main(int argc, char* argv[]) {
    int mode = -1;
    float p = 0.0;
    
    if (argc < 2) {
        printf("At least 2 arguments required \n");
        exit(1);
    }

    if (strcmp(argv[1], "1") == 0) {
        mode = 1;
    } else if (strcmp(argv[1], "n") == 0) {
        mode = 2;
    } else if (strcmp(argv[1], "p") == 0) {
        mode = 3;

        if (argc != 3) {
            printf("3 arguments required with this mode \n");
            exit(1);
        } 

        p = stof(argv[2]);
    } else {
        printf("Invalid option \n");
        exit(1);
    }

    PktGntr generator;
    Sender sender(p);

    if (mode == 1) {
        while(true) {
            sender.send1P(generator.generate());
        }
    } else if (mode == 2) {
        while(true) {
            sender.sendNP(generator.generate());
        }
    } else if (mode == 3) {
        while(true) {
            sender.sendPP(generator.generate());
        }
    }
}