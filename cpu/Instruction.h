#ifndef PSXEMU_INSTRUCTION_H
#define PSXEMU_INSTRUCTION_H

#include <cstdint>

struct RegisterIndex {
    uint32_t index;
};

class Instruction {
public:
    explicit Instruction(const uint32_t& opcode) {
        this->opcode = opcode;
    }
    uint32_t function() const;
    uint32_t subfunction() const;
    RegisterIndex t() const;
    RegisterIndex s() const;
    RegisterIndex d() const;
    uint32_t imm() const;
    uint32_t imm_se() const;
    uint32_t imm_shift() const;
    uint32_t imm_jump() const;
    uint32_t cop_opcode() const;

    uint32_t opcode;
};


#endif //PSXEMU_INSTRUCTION_H
