//
// Created by sven on 28.11.20.
//

#ifndef PSXEMU_CPU_H
#define PSXEMU_CPU_H

#include <cstdint>
#include "Interconnect.h"

class Cpu {
public:
    Interconnect* interconnect;
    Cpu(Interconnect* interconnect) {
        // PC reset value at the beginning of the BIOS
        this->pc = 0xbfc00000;

        // interconnect for peripherals
        this->interconnect = interconnect;
    }
    void runNextInstruction();

private:
    uint32_t load32(const uint32_t& address) const;
    void decodeAndExecute(const uint32_t& instruction);
    uint32_t pc; // Instruction Pointer (Program Counter)
};


#endif //PSXEMU_CPU_H
