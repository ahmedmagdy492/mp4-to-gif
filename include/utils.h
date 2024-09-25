#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <cstddef>
#include <cmath>

float GetFramesRepeatRatio(uint8_t *frame1, uint8_t *frame2, size_t len) {
    float noOfNonZeroBytes = 0;

    for(int i = 0;i < len; ++i) {
        int dif = frame1[i]-frame2[i];
        if(dif > 0) {
            ++noOfNonZeroBytes;
        } 
    }
    
    return noOfNonZeroBytes / len * 100.0f;
}

#endif