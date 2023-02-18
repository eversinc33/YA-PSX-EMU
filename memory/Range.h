#ifndef PSXEMU_RANGE_H
#define PSXEMU_RANGE_H

#include <cstdint>

class Range {
public:
    Range(uint32_t start, uint32_t length) {
        this->start = start;
        this->length = length;
    }
    bool contains(const uint32_t& address) const;

    uint32_t length;
    uint32_t start;
};


#endif //PSXEMU_RANGE_H
