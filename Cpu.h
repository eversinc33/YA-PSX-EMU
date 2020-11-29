//
// Created by sven on 28.11.20.
//

#ifndef PSXEMU_CPU_H
#define PSXEMU_CPU_H

#include <cstdint>
#include "Interconnect.h"
#include "Instruction.h"

struct LoadRegister {
    RegisterIndex registerIndex;
    uint32_t value;
};

enum Exception {
    SysCall = 0x8, // caused by syscall opcode
};

class Cpu {
public:
    Interconnect* interconnect;
    explicit Cpu(Interconnect* interconnect)
        : pc(0xbfc00000), // PC reset value at the beginning of the BIOS
          next_pc(0xbfc00000 + 4),
          sr(0),
          hi(0xdeadbeef),
          lo(0xdeadbeef),
          load({{0}, 0})
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
    void OP_LB(const Instruction &instruction);
    void OP_BEQ(const Instruction &instruction);
    void OP_AND(const Instruction &instruction);
    void OP_JALR(const Instruction &instruction);
    void OP_MFC0(const Instruction &instruction);
    void OP_ADD(const Instruction &instruction);
    void OP_BGTZ(const Instruction &instruction);
    void OP_BLEZ(const Instruction &instruction);
    void OP_LBU(const Instruction &instruction);
    void OP_BXX(const Instruction &instruction);
    void OP_SLTI(const Instruction &instruction);
    void OP_SUBU(const Instruction &instruction);
    void OP_SRA(const Instruction &instruction);
    void OP_DIV(const Instruction &instruction);
    void OP_MFLO(const Instruction &instruction);
    void OP_SRL(const Instruction &instruction);
    void OP_SLTIU(const Instruction &instruction);
    void OP_DIVU(const Instruction &instruction);
    void OP_MFHI(const Instruction &instruction);
    void OP_SLT(const Instruction &instruction);
    // coprocessor opcodes
    void OP_COP0(const Instruction &instruction);
    void OP_MTC0(const Instruction &instruction);

    // registers
    uint32_t pc; // Instruction Pointer (Program Counter)
    uint32_t current_pc; // save address of current instruction to savein EPC in case of exception
    uint32_t next_pc; // simulate branch delay slot
    uint32_t regs[32] = {}; // 32 general purpose registers
    uint32_t sr; // cop0 register 12: status register
    uint32_t hi; // hi register for divison remainder and multiplication high result
    uint32_t lo; // lo register for division quotient and multiplication low result
    uint32_t cause; // cop0 register 13: cause register
    uint32_t epc; // cop0 register : exception PC
    // custom registers
    uint32_t out_regs[32] = {}; // second set of registers to emulate the load delay slot accurately. contain output of the curren instruction
    LoadRegister load; // load initiated by the current instruction
    // get and set
    uint32_t getRegister(const RegisterIndex& t);
    void setRegister(const RegisterIndex& t, const uint32_t& v);

    unsigned int n_instructions = 0;

    void branch(uint32_t offset);
    void exception(Exception exception);
    uint8_t load8(const uint32_t &address) const;

    void OP_SYSCALL(const Instruction &instruction);

    void OP_MTLO(const Instruction &instruction);

    void OP_MTHI(const Instruction &instruction);

    void OP_NOR(const Instruction &instruction);

    void OP_RFE(const Instruction &instruction);
};


#endif //PSXEMU_CPU_H
