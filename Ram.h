//
// Created by sven on 28.11.20.
//

#ifndef PSXEMU_RAM_H
#define PSXEMU_RAM_H

#include <vector>
#include "Range.h"



class Ram {
public:
    const uint32_t START_ADDRESS = 0x00000000;
    const uint32_t SIZE = 2 * 1024 * 1024; // 2 MB

    std::vector<unsigned char> data;

    Ram() : range(Range(START_ADDRESS, SIZE)) {
        data = std::vector<unsigned char>(SIZE);
        std::fill(data.begin(), data.end(), 0xca); // fill with whatever value
    }

    Range range;

    uint32_t load32(const uint32_t &offset) const;
    void store32(const uint32_t &offset, const uint32_t& value);
};


#endif //PSXEMU_RAM_H
