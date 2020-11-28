//
// Created by sven on 28.11.20.
//

#include "Instruction.h"

// return bits 32:26 of the instruction
uint32_t Instruction::function() const {
    return this->opcode >> 26u;
}

// bits 5:0 describe the subfunction
uint32_t Instruction::subfunction() const {
    return this->opcode & 0x3fu;
}

// return register index in bits 15:11
uint32_t Instruction::d() const {
    return (this->opcode >> 11u) & 0x1fu;
}

// return register index in bits 20:16
uint32_t Instruction::t() const {
    return (this->opcode >> 16u) & 0x1fu;
}

// return immediate value index in bits 16:0
uint32_t Instruction::imm() const {
    return this->opcode & 0xffffu;
}

// return immediate value index in bits 16:0 as sign extended 32 bit value
uint32_t Instruction::imm_se() const {
    auto v = (int16_t) (this->opcode & 0xffffu);
    return (uint32_t) v;
}

// return shift immediate value, which is stored in 10:6
uint32_t Instruction::shift_imm() const {
    return (this->opcode >> 6u) & 0x1fu;
}

// return register index in bits 25:21
uint32_t Instruction::s() const {
    return (this->opcode >> 21u) & 0x1fu;
}
