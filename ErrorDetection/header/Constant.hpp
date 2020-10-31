#include <cstdint>
#include "iostream"

using namespace std;

typedef uint8_t     byte_t;

typedef uint32_t    crc_t;
typedef uint8_t     vrc_t;
typedef uint8_t     lrc_t;
typedef uint16_t    cksum_t;

typedef uint64_t    len_t;
typedef uint64_t    indx_t;
typedef uint64_t    cnt_t;
typedef uint64_t    semVal_t;

const string CHANNEL_NAME = "channel";
const string SEMBUFFEREMPTY_NAME = "sembufferempty";
const string SEMBUFFERFULL_NAME = "sembufferfull";
const indx_t TAINT_TIMES = 64;