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

    uint8_t load8(const uint32_t &offset);

    void store8(const uint32_t &offset, const uint8_t &value);

    void store16(const uint32_t &offset, const uint16_t &value);

    uint16_t load16(const uint32_t &offset);
};


#endif //PSXEMU_RAM_H
