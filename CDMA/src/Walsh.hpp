#pragma once
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <cstdio>
#include <vector>

using namespace std;

#include "Error.hpp"

class Walsh {
  vector<vector<int8_t>> codes;
  uint8_t lstId;
  int size;
  // Returns the encoded vector for a bit
  vector<int8_t> encode(uint8_t id, uint8_t bit);
  // Returns the decoded bit
  uint8_t decode(uint8_t id, const vector<int8_t> &encoded);

public:
  // N is the number of Senders
  Walsh(uint8_t N);
  // Returns size of Walsh code
  uint8_t getSize();
  // Return an id for the Walsh code
  uint8_t getId();

  // Encode the data
  vector<vector<int8_t>> encodeData(uint8_t id, const vector<uint8_t> &data);

  // Decode the data
  vector<uint8_t> decodeData(uint8_t id, const vector<vector<int8_t>> &data);
};

// Create Walsh Table
inline Walsh::Walsh(uint8_t N) {
  size = 1;

  // Calculate the size of Walsh Table
  while (size < N) {
    size *= 2;
  }

  // Resize codes table
  codes.resize(size);
  for (uint8_t i = 0; i < size; i++) {
    codes[i].resize(size);
  }

  codes[0][0] = 1;
  int currSize = 1;

  while (currSize < size) {
    for (int r = 1; r <= 2 * currSize; r++) {
      for (int c = 1; c <= 2 * currSize; c++) {
        if (r >= 1 && r <= currSize) {
          if (c >= 1 && c <= currSize) {
            continue;
          } else {
            codes[r - 1][c - 1] = codes[r - 1][c - currSize - 1];
          }
        } else {
          if (c >= 1 && c <= currSize) {
            codes[r - 1][c - 1] = codes[r - currSize - 1][c - 1];
          } else {
            codes[r - 1][c - 1] =
                -1 * codes[r - currSize - 1][c - currSize - 1];
          }
        }
      }
    }
    currSize *= 2;
  }

  lstId = 0;
}

inline uint8_t Walsh::getSize() { return size; }

inline uint8_t Walsh::getId() {
  if (lstId == codes.size()) {
    Error e;
    e.className = "Walsh";
    e.funcName = "getId";
    e.problem = "Id value exceeded";
    throw e;
  }

  uint8_t tmp = lstId;
  lstId++;
  return tmp;
}

inline vector<int8_t> Walsh::encode(uint8_t id, uint8_t bit) {
  vector<int8_t> encoded;
  encoded = codes[id];

  if (bit == 0) {
    for (int i = 0; i < encoded.size(); i++) {
      encoded[i] *= -1;
    }
  }

  return encoded;
}

inline uint8_t Walsh::decode(uint8_t id, const vector<int8_t> &encoded) {
  int val = 0;
  
  for (int i = 0; i < codes[id].size(); i++) {
    val += encoded[i] * codes[id][i];
  }

  if(val > size) {
    Error e;
    throw e;
  }

  val /= size;

  if (val == -1) {
    return 0;
  }

  if (val == 1) {
      return 1;
  }

  return 2;
}

inline vector<vector<int8_t>> Walsh::encodeData(uint8_t id, const vector<uint8_t> &data) {
    uint8_t tmpByte;

    vector<vector<int8_t>> encoded;
    for(int i = 0; i < data.size(); i++) {
        tmpByte = data[i];
        uint8_t maskVal = 1;
        for(uint8_t bitnum = 1; bitnum <= 8; bitnum++) {
            encoded.push_back(encode(id, maskVal&tmpByte));
            maskVal <<= 1;
        }
    }

    return encoded;
}

inline vector<uint8_t> Walsh::decodeData(uint8_t id, const vector<vector<int8_t>> &data) {
    uint8_t tmpByte;
    uint8_t tmp;
    vector<uint8_t> decoded;
    
    for(int i = 0; i < data.size();) {
        tmpByte = 0;
        for(uint8_t bitnum = 1; bitnum <= 8; bitnum++, i++) {
            tmp = decode(id, data[i]);

            if(tmp == 1) {
              tmpByte |= (1 << (bitnum-1));
            } else if (tmp == 2){
              decoded.clear(); return decoded;
            }
        }

        decoded.push_back(tmpByte);
    }

    return decoded;
}