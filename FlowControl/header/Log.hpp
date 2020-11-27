#include "Constant.hpp"
#include <cstdint>
#include <chrono>

using namespace std;

class Log{
    uint64_t numPackets;
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
};