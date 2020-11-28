//
// Created by sven on 28.11.20.
//

#include "Instruction.h"

// return bits 32:26 of the instruction
uint32_t Instruction::function() const {
    return this->opcode >> 26;
}

// return register index in bits 20:16
uint32_t Instruction::target() const {
    return (this->opcode >> 16) & 0x1f;
}

// return immediate value index in bits 16:0
uint32_t Instruction::imm() const {
    return this->opcode & 0xffff;
}

// return register index in bits 25:21
uint32_t Instruction::source() const {
    return (this->opcode >> 21) & 0x1f;
}
