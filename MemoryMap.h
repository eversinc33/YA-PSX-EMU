//
// Created by sven on 28.11.20.
//

#ifndef PSXEMU_MEMORYMAP_H
#define PSXEMU_MEMORYMAP_H

#include "Range.h"

// http://problemkaputt.de/psx-spx.htm#memorymap
const Range HARDWARE_REGISTERS = Range(0x1f801000, 36);
const Range RAM_SIZE_REGISTER = Range(0x1f801060, 4); // register that does some ram configuration, set by the bios, should be save to ignore

#endif //PSXEMU_MEMORYMAP_H