//
// Created by sven on 28.11.20.
//

#ifndef PSXEMU_INSTRUCTION_H
#define PSXEMU_INSTRUCTION_H


#include <cstdint>

class Instruction {
public:
    explicit Instruction(const uint32_t& opcode) {
        this->opcode = opcode;
    }
    uint32_t function() const;
    uint32_t target() const;
    uint32_t source() const;
    uint32_t imm() const;

    uint32_t opcode;
};


#endif //PSXEMU_INSTRUCTION_H
