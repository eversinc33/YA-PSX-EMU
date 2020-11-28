//
// Created by sven on 28.11.20.
//

#include <iostream>
#include <bitset>
#include "Cpu.h"
#include "Instruction.h"

void Cpu::runNextInstruction() {

    // Fetch instruction at PC (IP)
    uint32_t instruction = this->load32(this->pc);

    std::cout << std::endl << "$8: " << std::hex << this->getRegister(8) << std::endl;
    std::cout << "$PC: " << std::hex << this->pc << std::endl;
    std::cout << "Next instruction: " << std::hex << instruction << " | " << std::bitset<8>(Instruction(instruction).function()) << std::endl;

    this->decodeAndExecute(instruction);

    // Increment PC to point to the next instruction. (each is 32 bit)
    this->pc += 4;
}

uint32_t Cpu::load32(const uint32_t& address) const {
    return this->interconnect->load32(address);
}

void Cpu::decodeAndExecute(const uint32_t &instruction) {

    Instruction i = Instruction(instruction);
    switch(i.function()) {
        case 0b001111: // http://problemkaputt.de/psx-spx.htm#cpuspecifications
            this->OP_LUI(i);
            break;
        case 0b001101: // http://mipsconverter.com/opcodes.html
            this->OP_ORI(i);
            break;
        default:
            std::cout << "Unhandled instruction" << std::endl;
            throw std::exception();
    }
}


uint32_t Cpu::getRegister(const uint32_t &t) {
    if (t == 0) {
        return 0; // r0 is always zero
    }
    return this->regs[t];
}

void Cpu::setRegister(const uint32_t &t, const uint32_t &v) {
    std::cout << "Loading (in big endian) " << v << " into register " << t << std::endl;
    this->regs[t] = v;
}

// load upper immediate opcode:
// load value 'immediate' into upper 16 bits of target
void Cpu::OP_LUI(const Instruction &instruction) {
    uint32_t immediate = instruction.imm();
    uint32_t target = instruction.target();

    uint32_t value = immediate << 16; // low 16bit set to 0

    this->setRegister(target, value);
}

// bitwise or opcode:
// bitwise or of value 'immediate' with source into target
void Cpu::OP_ORI(const Instruction &instruction) {
    uint32_t immediate = instruction.imm();
    uint32_t target = instruction.target();
    uint32_t source = instruction.source();

    uint32_t value = this->getRegister(source) | immediate;

    this->setRegister(target, value);
}
