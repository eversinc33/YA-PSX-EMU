//
// Created by sven on 28.11.20.
//

#ifndef PSXEMU_BIOS_H
#define PSXEMU_BIOS_H


#include <cstdint>
#include <bits/types/FILE.h>
#include <cstdio>
#include <fstream>
#include "Range.h"

class Bios {
public:
    const uint32_t START_ADDRESS = 0x1fc00000;

    Bios(const char *fname, const uint32_t& buffersize) : range(Range(START_ADDRESS, buffersize)) {
        this->readBinary(fname, buffersize);
    }

    uint32_t load32(const uint32_t& offset) const;

    Range range;
    unsigned char *data = nullptr;

    uint8_t load8(const uint32_t &offset) const;

private:
    void readBinary(const char *string, const uint32_t &i);
};


#endif //PSXEMU_BIOS_H
