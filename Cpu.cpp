//
// Created by sven on 28.11.20.
//

#include <iostream>
#include "Cpu.h"

void Cpu::runNextInstruction() {

    // Fetch instruction at PC (IP)
    uint32_t instruction = this->load32(this->pc);

    std::cout << instruction << std::endl;

    this->decodeAndExecute(instruction);

    // Increment PC to point to the next instruction.
    this->pc += 4;
}

uint32_t Cpu::load32(const uint32_t& address) const {
    return this->interconnect->load32(address);
}

void Cpu::decodeAndExecute(const uint32_t &instruction) {
    std::cout << std::hex << instruction << std::endl;
    throw std::exception();
}
