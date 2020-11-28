//
// Created by sven on 28.11.20.
//

#ifndef PSXEMU_INTERCONNECT_H
#define PSXEMU_INTERCONNECT_H


#include "Bios.h"
#include "Ram.h"

// http://problemkaputt.de/psx-spx.htm#memorymap
const Range HARDWARE_REGISTERS = Range(0x1f801000, 36);
const Range RAM_SIZE_REGISTER = Range(0x1f801060, 4); // register that does some ram configuration, set by the bios, should be save to ignore
const Range CACHE_CONTROL = Range(0xfffe0130, 4);

class Interconnect {
public:
    Bios* bios;
    Ram *ram;

    Interconnect(Bios* bios, Ram* ram) {
        this->bios = bios;
        this->ram = ram;
    };

    uint32_t load32(const uint32_t& address) const;
    void store32(const uint32_t& address, const uint32_t& value);
};


#endif //PSXEMU_INTERCONNECT_H
