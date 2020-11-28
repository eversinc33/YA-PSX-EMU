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
    Cpu(Interconnect* interconnect)
        : nextInstruction(0x0), // NOP
          pc(0xbfc00000) // PC reset value at the beginning of the BIOS
    {
        // set general purpose registers to default value
        for (uint32_t& reg : this->regs) {
            reg = 0xdeadbeef;
        }
        // R0 is hardwired to 0
        this->regs[0] = 0;

        // memory interface: interconnect for peripherals
        this->interconnect = interconnect;
    }
    void runNextInstruction();

private:
    void store32(const uint32_t& address, const uint32_t& value) const;
    uint32_t load32(const uint32_t& address) const;

    void decodeAndExecute(const Instruction &instruction);
    // opcodes
    void OP_LUI(const Instruction& instruction);
    void OP_ORI(const Instruction& instruction);
    void OP_SW(const Instruction &instruction);
    void OP_SLL(const Instruction &instruction);
    void OP_ADDIU(const Instruction &instruction);
    void OP_J(const Instruction &instruction);
    void OP_OR(const Instruction &instruction);

    // registers
    uint32_t pc; // Instruction Pointer (Program Counter)
    uint32_t regs[32] = {}; // 32 general purpose registers

    uint32_t getRegister(const uint32_t& t);

    void setRegister(const uint32_t& t, const uint32_t& v);

    Instruction nextInstruction;

    unsigned int n_instructions = 0;
};


#endif //PSXEMU_CPU_H
