#include "Constant.hpp"

#include "iostream"
#include "iomanip"

using namespace std;

class Log {
    cnt_t logCnt;
    cnt_t numPackets;

    cnt_t vrcError;
    cnt_t lrcError;
    cnt_t cksumError;
    cnt_t crcError;
public:
    static const byte_t NONE_ER = 0x0;
    static const byte_t VRC_ER = 0x1;
    static const byte_t LRC_ER = VRC_ER<<1;
    static const byte_t CKSUM_ER = LRC_ER<<1;
    static const byte_t CRC_ER = CKSUM_ER<<1;

    Log() {        
        logCnt = 0;
        numPackets = 0;
        vrcError = 0;
        crcError = 0;
        lrcError = 0;
        cksumError = 0;
    }

    void write(byte_t untaintedData[], byte_t taintedData[], len_t len, byte_t errorType) {
        numPackets++;
        if(errorType == NONE_ER) {
            return;
        }
        
        logCnt++;
        cout << "-------------" << logCnt << "-------------\n";
        
        if(VRC_ER&errorType) {
            vrcError++;
            cout << "VRC failed - " << vrcError << " out of " << numPackets << "\n";
        }

        if(VRC_ER&errorType) {
            lrcError++;
            cout << "LRC failed - " << lrcError << " out of " << numPackets << "\n";
        }

        if(CKSUM_ER&errorType) {
            cksumError++;
            cout << "CkSum failed - " << cksumError << " out of " << numPackets << "\n";
        }

        if(CRC_ER&errorType) {
            crcError++;
            cout << "CRC failed - " << crcError << " out of " << numPackets << "\n";
        }

        for(indx_t i = 0; i < len; i++) {
            cout << setw(2) << hex << untaintedData[i];
        }

        for(indx_t i = 0; i < len; i++) {
            cout << setw(2) << hex << taintedData[i];
        }

        byte_t tmp;

        for(indx_t i = 0; i < len; i++) {
            tmp = taintedData[i]^untaintedData[i];

            if(tmp) {
                if(tmp>>4) {
                    cout << "^-";
                } else {
                    cout << "-^";
                }
            } else {
                cout << "--";
            }
        }

        cout << "\n" << "-------------" << logCnt << "-------------\n";
    }
};