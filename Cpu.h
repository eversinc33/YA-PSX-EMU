//
// Created by sven on 28.11.20.
//

#ifndef PSXEMU_CPU_H
#define PSXEMU_CPU_H

#include <cstdint>
#include "Interconnect.h"
#include "Instruction.h"

class Cpu {
public:
    Interconnect* interconnect;
    Cpu(Interconnect* interconnect) {
        // set general purpose registers to default value
        for (uint32_t& reg : this->regs) {
            reg = 0xdeadbeef;
        }
        // R0 is hardwired to 0
        this->regs[0] = 0;

        // PC reset value at the beginning of the BIOS
        this->pc = 0xbfc00000;

        // memory interface: interconnect for peripherals
        this->interconnect = interconnect;
    }
    void runNextInstruction();
    uint32_t getRegister(const uint32_t& t);
    void setRegister(const uint32_t& t, const uint32_t& v);


private:
    uint32_t load32(const uint32_t& address) const;
    void decodeAndExecute(const uint32_t& instruction);

    // Instruction Pointer (Program Counter)
    uint32_t pc;
    // 32 general purpose registers, set to deadbeef by default
    uint32_t regs[32] = {};

    // opcodes
    void OP_LUI(const Instruction& instruction);
    void OP_ORI(const Instruction& instruction);
};


#endif //PSXEMU_CPU_H
