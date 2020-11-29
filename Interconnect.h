//
// Created by sven on 28.11.20.
//

#ifndef PSXEMU_INTERCONNECT_H
#define PSXEMU_INTERCONNECT_H


#include "Bios.h"
#include "Ram.h"

// KUSEG, KSEG etc. all refer to the same address space, so convert them to real addresses,
// by masking their region bits.
// the mask is selected using the 3 MSB of the address, so each entry matches 512KB of address space.
const uint32_t REGION_MASK[] = {
        // KUSEG: 2048 MB
        0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
        // KSEG0: 512 MB
        0x7fffffff,
        // KSEG1: 512 MB
        0x1fffffff,
        // KSEG2: 1024 MB
        0xffffffff, 0xffffffff
};

// http://problemkaputt.de/psx-spx.htm#memorymap
const Range HARDWARE_REGISTERS = Range(0x1f801000, 36);
const Range RAM_SIZE_REGISTER = Range(0x1f801060, 4); // register that does some ram configuration, set by the bios, should be save to ignore
const Range CACHE_CONTROL = Range(0xfffe0130, 4);
const Range SPU = Range(0x1f801c00, 640);
const Range EXPANSION_1 = Range(0x1f000000, 512 * 1024);
const Range EXPANSION_2 = Range(0x1f802000, 66);

class Interconnect {
public:
    Bios* bios;
    Ram *ram;

    Interconnect(Bios* bios, Ram* ram) {
        this->bios = bios;
        this->ram = ram;
    };

    uint32_t load32(const uint32_t& address);
    void store32(const uint32_t& address, const uint32_t& value);
    void store16(const uint32_t &address, const uint16_t &value);
    void store8(const uint32_t &address, const uint8_t &value);

    uint8_t load8(const uint32_t& address);

private:
    uint32_t maskRegion(const uint32_t& address);
};


#endif //PSXEMU_INTERCONNECT_H
