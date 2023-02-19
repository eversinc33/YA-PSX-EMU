#ifndef CHANNEL_H
#define CHANNEL_H

#pragma once

#include <stdint.h>
#include "../util/logging.h"

enum Direction {
    ToRam = 0,
    FromRam = 1,
};

enum Step {
    Increment = 0,
    Decrement = 1,
};

enum Sync {
    Manual = 0, // transfer starts when CPU writes to the trigger bit and everything is transfered at once
    Request = 1, // sync blocks to DMA requests
    LinkedList = 2, // used to transfer GPU command lists
};

// Channel ports
enum Port {
    MdecIn = 0, // Macroblock decoder input
    MdecOut = 1, // Macroblock decoder output
    Gpu_port = 2,
    CdRom = 3, 
    Spu = 4,
    Pio = 5, // Extension port
    Otc = 6, // used to clear the ordering table of the GTE
};

class Channel
{
public:
    uint32_t base; // channels base address: address of first word to be read/written from/to RAM
    Direction direction;

    Channel() : enable(false), direction(ToRam), step(Increment), sync(Manual), trigger(false), chop(false), chopDmaSz(0), chopCpuSz(0), dummy(0), base(0) {
    };
    ~Channel() {
        DEBUG("DESTRUCTED");
    };

    uint32_t getControl() const;
    void setControl(const uint32_t &value);
    void setBase(const uint32_t &value);
    uint32_t getBlockControl();
    void setBlockControl(const uint32_t &value);
    bool isActive() const;
    Sync getSyncMode() const;
    Step getStepMode() const;
    uint32_t getTransferSize() const;
    void done();

private:
    bool enable;
    Step step;
    Sync sync;
    bool trigger; // triggers DMA transfer if 'sync' is 'Manual'
    bool chop; // if true, the DMA chops the transfer and lets the cpu run inbetween
    uint8_t chopDmaSz; // chopping DMA window size (log2 number of words)
    uint8_t chopCpuSz; // choppung CPU window size (log2 number of cycles)
    uint8_t dummy; // unkown 2 rw bits in config register
    // 32 bit block control register
    uint16_t blockSize; // Size of a block in words
    uint16_t blockCount; // Block count, only used when sync is Request
};

#endif