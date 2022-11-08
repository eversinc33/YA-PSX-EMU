#include <iostream>
#include <bitset>
#include "Cpu.h"
#include "Instruction.h"

void Cpu::runNextInstruction() {

    if (this->pc % 4 != 0) {
        return this->exception(LoadAddressError);
    }

    // emulate branch delay slot: execute instruction, already fetch next instruction at PC (IP)
    Instruction instruction = Instruction(this->load32(this->pc));

    // if the last instruction was a branch, were in the delay slot
    this->inDelaySlot = this->branching;
    this->branching = false;

    // Increment PC to point to the next instruction. (each is 32 bit)
    this->current_pc = this->pc;
    this->pc = this->next_pc;
    this->next_pc = this->next_pc + 4;

    // emulate branch delay slot: execute pending loads, if there are none, load $zero which is NOP
    this->setRegister(this->load.registerIndex, this->load.value);
    this->load = {{0}, 0}; // reset load register

    // debug
    this->n_instructions++;
    /*
    std::cout << std::endl << std::dec << "#" << n_instructions << std::endl;
    std::cout << "$12: " << std::hex << this->getRegister({0xa1}) << std::endl;
    std::cout << "$PC: " << std::hex << this->pc << std::endl;
    std::cout << "Next instruction: " << std::hex << instruction.opcode << "/" << std::bitset<8>(instruction.function()) << std::endl;
    */

    // execute next instrudction
    this->decodeAndExecute(instruction);

    // copy to actual registers
    std::copy(std::begin(out_regs), std::end(out_regs), std::begin(regs));
}

uint16_t Cpu::load16(uint32_t address) const {
    return this->interconnect->load16(address);
}

uint32_t Cpu::load32(const uint32_t& address) const {
    return this->interconnect->load32(address);
}

void Cpu::store8(const uint32_t &address, const uint8_t &value) const {
    this->interconnect->store8(address, value);
}

void Cpu::store16(const uint32_t &address, const uint16_t &value) const {
    this->interconnect->store16(address, value);
}

void Cpu::store32(const uint32_t &address, const uint32_t &value) const {
    this->interconnect->store32(address, value);
}

void Cpu::decodeAndExecute(const Instruction& instruction) {

    if (this->current_pc == 0x80000080) {
        //this->DEBUG = true;
    }
    if (this->DEBUG) {
        std::cout << "opcode: " << std::hex << instruction.opcode << "/" << std::bitset<8>(instruction.function()) << std::endl;
        std::cout << "pc: " << this->current_pc << std::endl;
        getchar();
    }

    switch(instruction.function()) {
        // http://mipsconverter.com/opcodes.html
        // http://problemkaputt.de/psx-spx.htm#cpuspecifications
        case 0b000000:
            switch (instruction.subfunction()) {
                case 0b000000: this->OP_SLL(instruction); break;
                case 0b000010: this->OP_SRL(instruction); break;
                case 0b000011: this->OP_SRA(instruction); break;
                case 0b000100: this->OP_SLLV(instruction); break;
                case 0b000110: this->OP_SRLV(instruction); break;
                case 0b000111: this->OP_SRAV(instruction); break;
                case 0b001000: this->OP_JR(instruction); break;
                case 0b001001: this->OP_JALR(instruction); break;
                case 0b001100: this->OP_SYSCALL(instruction); break;
                case 0b001101: this->OP_BREAK(instruction); break;
                case 0b010000: this->OP_MFHI(instruction); break;
                case 0b010001: this->OP_MTHI(instruction); break;
                case 0b010010: this->OP_MFLO(instruction); break;
                case 0b010011: this->OP_MTLO(instruction); break;
                case 0b011000: this->OP_MULT(instruction); break;
                case 0b011001: this->OP_MULTU(instruction); break;
                case 0b011010: this->OP_DIV(instruction); break;
                case 0b011011: this->OP_DIVU(instruction); break;
                case 0b100000: this->OP_ADD(instruction); break;
                case 0b100001: this->OP_ADDU(instruction); break;
                case 0b100010: this->OP_SUB(instruction); break;
                case 0b100011: this->OP_SUBU(instruction); break;
                case 0b100100: this->OP_AND(instruction); break;
                case 0b100101: this->OP_OR(instruction); break;
                case 0b100110: this->OP_XOR(instruction); break;
                case 0b100111: this->OP_NOR(instruction); break;
                case 0b101010: this->OP_SLT(instruction); break;
                case 0b101011: this->OP_SLTU(instruction); break;
                default: this->OP_ILLEGAL(instruction);
            }
            break;
        case 0b000001: this->OP_BXX(instruction); break;
        case 0b000010: this->OP_J(instruction); break;
        case 0b000011: this->OP_JAL(instruction); break;
        case 0b000100: this->OP_BEQ(instruction); break;
        case 0b000101: this->OP_BNE(instruction); break;
        case 0b000110: this->OP_BLEZ(instruction); break;
        case 0b000111: this->OP_BGTZ(instruction); break;
        case 0b001000: this->OP_ADDI(instruction); break;
        case 0b001001: this->OP_ADDIU(instruction); break;
        case 0b001010: this->OP_SLTI(instruction); break;
        case 0b001011: this->OP_SLTIU(instruction); break;
        case 0b001100: this->OP_ANDI(instruction); break;
        case 0b001101: this->OP_ORI(instruction); break;
        case 0b001110: this->OP_XORI(instruction); break;
        case 0b001111: this->OP_LUI(instruction); break;
        case 0b010000: this->OP_COP0(instruction); break;
        case 0b010001: this->OP_COP1(instruction); break;
        case 0b010010: this->OP_COP2(instruction); break;
        case 0b010011: this->OP_COP3(instruction); break;
        case 0b100000: this->OP_LB(instruction); break;
        case 0b100001: this->OP_LH(instruction); break;
        case 0b100010: this->OP_LWL(instruction); break;
        case 0b100011: this->OP_LW(instruction); break;
        case 0b100100: this->OP_LBU(instruction); break;
        case 0b100101: this->OP_LHU(instruction); break;
        case 0b100110: this->OP_LWR(instruction); break;
        case 0b101000: this->OP_SB(instruction); break;
        case 0b101001: this->OP_SH(instruction); break;
        case 0b101010: this->OP_SWL(instruction); break;
        case 0b101011: this->OP_SW(instruction); break;
        case 0b101110: this->OP_SWR(instruction); break;
        case 0b110000: this->OP_LWC0(instruction); break;
        case 0b110001: this->OP_LWC1(instruction); break;
        case 0b110010: this->OP_LWC2(instruction); break;
        case 0b110011: this->OP_LWC3(instruction); break;
        case 0b111000: this->OP_SWC0(instruction); break;
        case 0b111001: this->OP_SWC1(instruction); break;
        case 0b111010: this->OP_SWC2(instruction); break;
        case 0b111011: this->OP_SWC3(instruction); break;
        default: this->OP_ILLEGAL(instruction);
    }
}

uint32_t Cpu::getRegister(const RegisterIndex &t) {
    return this->regs[t.index];
}

void Cpu::setRegister(const RegisterIndex &t, const uint32_t &v) {
    this->out_regs[0] = 0; // r0 is always zero
    this->out_regs[t.index] = v;
}

void Cpu::exception(Exception exception) {
    // exception handler address depends on the BEV bit
    auto handler = (this->sr & (1u << 22u)) != 0 ? 0xbfc00180 : 0x80000080;

    // shift bits 5:0 of the status register (SR) two to the left
    // by shifting these, the core is put into kernel mode
    auto mode = this->sr & 0x3fu;
    this->sr &= ~0x3fu;
    this->sr |= (mode << 2u) & 0x3fu;

    // update cause register with bits 6:2 (the exception code)
    this->cause = ((uint32_t) exception) << 2u;

    // save current instruction address in EPC
    this->epc = this->current_pc;

    // special case, if the exception occurs in the delay slot
    if (this->inDelaySlot) {
        this->epc = this->epc - 4;
        // also set bit 31 of cause register
        this->cause |= 1u << 31u;
    }

    // no branch delay in exceptions!
    this->pc = handler;
    this->next_pc = this->pc + 4;
}
