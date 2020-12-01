//
// Created by sven on 01.12.20.
//

#ifndef PSXEMU_DMA_H
#define PSXEMU_DMA_H


#include <cstdint>
#include "Range.h"

class Dma {
public:
    uint32_t control; // DMA control register

    // dma interrupt register, unpacked into variables
    bool irqEnable; // master IRQ enable
    uint8_t channelIrqEnable;
    uint8_t channelIrqFlags;
    uint8_t forceIrq; // if set, interrupt is always active
    uint8_t irqDummy; // not sure what these bits do

    const Range range = Range(0x1f801080,0x80); // dma, direct memory access

    Dma() : control(0x07654321) // reset value, see no$ psx spec
    {

    }

    bool irq() const;
    uint32_t getInterrupt();
    void setInterrupt(uint32_t val);
    void setControl(const uint32_t &value);
};


#endif //PSXEMU_DMA_H
