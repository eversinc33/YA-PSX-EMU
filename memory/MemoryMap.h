//
// Created by sven on 01.12.20.
//

#ifndef PSXEMU_MEMORYMAP_H
#define PSXEMU_MEMORYMAP_H

#include "Range.h"

// http://problemkaputt.de/psx-spx.htm#memorymap
const Range HARDWARE_REGISTERS = Range(0x1f801000, 36);
const Range RAM_SIZE_REGISTER = Range(0x1f801060, 4); // register that does some ram configuration, set by the bios, should be save to ignore
const Range CACHE_CONTROL = Range(0xfffe0130, 4);
const Range SPU = Range(0x1f801c00, 640);
const Range EXPANSION_1 = Range(0x1f000000, 512 * 1024);
const Range EXPANSION_2 = Range(0x1f802000, 66);
const Range IRQ_CONTROL = Range(0x1f801070, 8); // interrupt control registers (status and mask)
const Range TIMERS = Range(0x1f801100,48); // the playstation has three independent timers at these regsters
const Range GPU = Range(0x1f801810, 8);

#endif //PSXEMU_MEMORYMAP_H
