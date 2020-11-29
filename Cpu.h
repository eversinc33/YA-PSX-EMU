//
// Created by sven on 28.11.20.
//

#ifndef PSXEMU_CPU_H
#define PSXEMU_CPU_H

#include <cstdint>
#include "Interconnect.h"
#include "Instruction.h"

struct LoadRegister {
    uint32_t index;
    uint32_t value;
};

class Cpu {
public:
    Interconnect* interconnect;
    explicit Cpu(Interconnect* interconnect)
        : nextInstruction(0x0), // NOP
          pc(0xbfc00000), // PC reset value at the beginning of the BIOS
          sr(0),
          load({0, 0})
    {
        // set general purpose registers to default value
        for (uint32_t& reg : this->regs) {
            reg = 0xdeadbeef;
        }
        // R0 is hardwired to 0
        this->regs[0] = 0;
        // out regs mirror the regs
        std::copy(std::begin(regs), std::end(regs), std::begin(out_regs));

        // memory interface: interconnect for peripherals
        this->interconnect = interconnect;
    }
    void runNextInstruction();

private:
    void store8(const uint32_t &address, const uint8_t &value) const;
    void store16(const uint32_t& address, const uint16_t & value) const;
    void store32(const uint32_t& address, const uint32_t& value) const;
    uint32_t load32(const uint32_t& address) const;

    void decodeAndExecute(const Instruction &instruction);
    // opcodes
    void OP_LUI(const Instruction& instruction);
    void OP_ORI(const Instruction& instruction);
    void OP_SW(const Instruction &instruction);
    void OP_LW(const Instruction &instruction);
    void OP_SLL(const Instruction &instruction);
    void OP_ADDIU(const Instruction &instruction);
    void OP_ADDI(const Instruction &instruction);
    void OP_ADDU(const Instruction &instruction);
    void OP_J(const Instruction &instruction);
    void OP_OR(const Instruction &instruction);
    void OP_BNE(const Instruction &instruction);
    void OP_SLTU(const Instruction &instruction);
    void OP_SH(const Instruction &instruction);
    void OP_JAL(const Instruction &instruction);
    void OP_ANDI(const Instruction &instruction);
    void OP_SB(const Instruction &instruction);
    void OP_JR(const Instruction &instruction);
    // coprocessor opcodes
    void OP_COP0(const Instruction &instruction);
    void OP_MTC0(const Instruction &instruction);

    // registers
    uint32_t pc; // Instruction Pointer (Program Counter)
    uint32_t regs[32] = {}; // 32 general purpose registers
    uint32_t sr; // cop0 register 12: status register
    // custom registers
    uint32_t out_regs[32] = {}; // second set of registers to emulate the load delay slot accurately. contain output of the curren instruction
    LoadRegister load; // load initiated by the current instruction
    // get and set
    uint32_t getRegister(const uint32_t& t);
    void setRegister(const uint32_t& t, const uint32_t& v);

    Instruction nextInstruction;

    unsigned int n_instructions = 0;

    void branch(uint32_t offset);

    void OP_LB(const Instruction &instruction);

    uint8_t load8(const uint32_t &address) const;

    void OP_BEQ(const Instruction &instruction);

    void OP_JALR(const Instruction &instruction);
};


#endif //PSXEMU_CPU_H
