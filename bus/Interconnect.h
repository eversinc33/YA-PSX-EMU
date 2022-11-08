#ifndef PSXEMU_INTERCONNECT_H
#define PSXEMU_INTERCONNECT_H

#include "../bios/Bios.h"
#include "../memory/Ram.h"
#include "../memory/Dma.h"

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

class Interconnect {
public:
    Bios* bios;
    Ram *ram;
    Dma *dma;

    Interconnect(Bios* bios, Ram* ram, Dma* dma) {
        this->bios = bios;
        this->ram = ram;
        this->dma = dma;
    };

    uint32_t load32(const uint32_t& address);
    void store32(const uint32_t& address, const uint32_t& value);
    void store16(const uint32_t &address, const uint16_t &value);
    void store8(const uint32_t &address, const uint8_t &value);

    uint8_t load8(const uint32_t& address);

    uint16_t load16(uint32_t address);

private:
    uint32_t maskRegion(const uint32_t& address);

    void doDma(const Port &port);
    void doDmaBlock(const Port &port);
    void doDmaLinkedList(const Port &port);
};


#endif //PSXEMU_INTERCONNECT_H
