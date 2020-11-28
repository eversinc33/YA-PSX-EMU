//
// Created by sven on 28.11.20.
//

#include <iostream>
#include <bitset>
#include "Cpu.h"
#include "Instruction.h"

void Cpu::runNextInstruction() {
    // emulate branch delay slot
    Instruction instruction = nextInstruction;

    // Fetch instruction at PC (IP)
    this->nextInstruction = Instruction(this->load32(this->pc));

    this->n_instructions++;
    std::cout << std::endl << std::dec << "Instruction " << n_instructions << std::endl;
    std::cout << "$8: " << std::hex << this->getRegister(8) << std::endl;
    std::cout << "$PC: " << this->pc << std::endl;
    std::cout << "Next instruction: " << std::hex << instruction.opcode << " | " << std::bitset<8>(instruction.function()) << std::endl;

    // Increment PC to point to the next instruction. (each is 32 bit)
    this->pc += 4;

    this->decodeAndExecute(instruction);
}

uint32_t Cpu::load32(const uint32_t& address) const {
    return this->interconnect->load32(address);
}

void Cpu::store32(const uint32_t &address, const uint32_t &value) const {
    this->interconnect->store32(address, value);
}

void Cpu::decodeAndExecute(const Instruction& instruction) {

    switch(instruction.function()) {
        // http://mipsconverter.com/opcodes.html
        // http://problemkaputt.de/psx-spx.htm#cpuspecifications
        case 0b001111:
            this->OP_LUI(instruction);
            break;
        case 0b001101:
            this->OP_ORI(instruction);
            break;
        case 0b101011:
            this->OP_SW(instruction);
            break;
        case 0b000000:
            switch (instruction.subfunction()) {
                case 0b000000:
                    this->OP_SLL(instruction);
                    break;
                case 0b100101:
                    this->OP_OR(instruction);
                    break;
                default:
                    std::cout << "Unhandled instruction" << std::endl;
                    throw std::exception();
            }
        case 0b001001:
            this->OP_ADDIU(instruction);
            break;
        case 0b000010:
            this->OP_J(instruction);
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
void Cpu::OP_LUI(const Instruction& instruction) {
    auto immediate = instruction.imm();
    auto t = instruction.t();

    auto value = immediate << 16u; // low 16bit set to 0

    this->setRegister(t, value);
}

// bitwise or opcode:
// bitwise or of value 'immediate' with source into target
void Cpu::OP_ORI(const Instruction& instruction) {
    auto immediate = instruction.imm();
    auto t = instruction.t();
    auto s = instruction.s();

    auto value = this->getRegister(s) | immediate;

    this->setRegister(t, value);
}

// store word opcode:
// store the word in target in source plus memory offset of immediate
void Cpu::OP_SW(const Instruction& instruction) {
    auto immediate = instruction.imm_se(); // SW is sign extending
    auto t = instruction.t();
    auto s = instruction.s();

    auto address = this->getRegister(s) + immediate;
    auto value = this->getRegister(t);

    this->store32(address, value);
}

// shift left logical
// shift bits from target by immediate to the left and store in destination
void Cpu::OP_SLL(const Instruction& instruction) {
    auto immediate = instruction.imm_shift();
    auto t = instruction.t();
    auto d = instruction.d();

    auto value = this->getRegister(t) << immediate;
    this->setRegister(d, value);
}

// add immediate unsigned
// name is misleading:
// we simply add immediate to source, save in target and truncate result on overflow
void Cpu::OP_ADDIU(const Instruction& instruction) {
    auto immediate = instruction.imm_se();
    auto t = instruction.t();
    auto s = instruction.s();

    auto value = this->getRegister(s) + immediate;
    this->setRegister(t, value);
}

// jump
// set PC (instruction pointer) to address in immediate
void Cpu::OP_J(const Instruction& instruction) {
    auto immediate = instruction.imm_jump();

    // immediate is shifted 2 to the right, because the two LSBs of pc are always zero anyway (due to the 32bit boundary)
    this->pc = (this->pc & 0xf0000000u) | (immediate << 2u);
    std::cout << "Jumping to: " << this->pc << std::endl;
}

// or
// bitwise or
void Cpu::OP_OR(const Instruction &instruction) {
    auto s = instruction.s();
    auto t = instruction.t();
    auto d = instruction.d();

    auto value = this->getRegister(s) | this->getRegister(t);
    this->setRegister(d, value);
}
