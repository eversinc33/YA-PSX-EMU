//
// Created by sven on 28.11.20.
//

#include <iostream>
#include <bitset>
#include <tiff.h>
#include "Cpu.h"
#include "Instruction.h"

void Cpu::runNextInstruction() {

    if (this->pc % 4 != 0) {
        std::cout << "pc_not_aligned_with_32bit!" << std::endl;
        throw std::exception();
    }

    // emulate branch delay slot:
    // -> execute instruction, already fetch next instruction at PC (IP)
    Instruction instruction = nextInstruction;
    this->nextInstruction = Instruction(this->load32(this->pc));

    // -> execute pending loads, if there are none, load $zero which is NOP
    if (this->load.registerIndex.index != 0) {
        std::cout << std::endl << "delayed load: ";
        this->setRegister(this->load.registerIndex, this->load.value);
        this->load = {0, 0}; // reset load register
    }

    // debug
    this->n_instructions++;
    std::cout << std::endl << std::dec << "#" << n_instructions << " | ";
    /*std::cout << "$12: " << std::hex << this->getRegister(8) << std::endl;*/
    std::cout << "$PC: " << this->pc << std::endl;
    std::cout << "Next instruction: " << std::hex << instruction.opcode << "/" << std::bitset<8>(instruction.function()) << std::endl;

    this->pc = this->pc + 4; // Increment PC to point to the next instruction. (each is 32 bit)

    this->decodeAndExecute(instruction);

    // copy to actual registers
    std::copy(std::begin(out_regs), std::end(out_regs), std::begin(regs));
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

    switch(instruction.function()) {
        // http://mipsconverter.com/opcodes.html
        // http://problemkaputt.de/psx-spx.htm#cpuspecifications
        case 0b001111:
            std::cout << "OP_LUI" << std::endl;
            this->OP_LUI(instruction);
            break;
        case 0b001101:
            std::cout << "OP_ORI" << std::endl;
            this->OP_ORI(instruction);
            break;
        case 0b101011:
            std::cout << "OP_SW" << std::endl;
            this->OP_SW(instruction);
            break;
        case 0b000000:
            std::cout << "000000_opcode:" << std::bitset<8>(instruction.subfunction()) << std::endl;
            switch (instruction.subfunction()) {
                case 0b000000:
                    std::cout << "OP_SLL" << std::endl;
                    this->OP_SLL(instruction);
                    break;
                case 0b100101:
                    std::cout << "OP_OR" << std::endl;
                    this->OP_OR(instruction);
                    break;
                case 0b101011:
                    std::cout << "OP_SLTU" << std::endl;
                    this->OP_SLTU(instruction);
                    break;
                case 0b100001:
                    std::cout << "OP_ADDU" << std::endl;
                    this->OP_ADDU(instruction);
                    break;
                case 0b001000:
                    std::cout << "OP_JR" << std::endl;
                    this->OP_JR(instruction);
                    break;
                default:
                    std::cout << "Unhandled_000000_opcode:" << std::bitset<8>(instruction.subfunction()) << std::endl;
                    throw std::exception();
            }
            break;
        case 0b001001:
            std::cout << "OP_ADDIU" << std::endl;
            this->OP_ADDIU(instruction);
            break;
        case 0b001000:
            std::cout << "OP_ADDI" << std::endl;
            this->OP_ADDI(instruction);
            break;
        case 0b000010:
            std::cout << "OP_J" << std::endl;
            this->OP_J(instruction);
            break;
        case 0b000101:
            std::cout << "OP_BNE" << std::endl;
            this->OP_BNE(instruction);
            break;
        case 0b100011:
            std::cout << "OP_LW" << std::endl;
            this->OP_LW(instruction);
            break;
        case 0b101001:
            std::cout << "OP_SH" << std::endl;
            this->OP_SH(instruction);
            break;
        case 0b000011:
            std::cout << "OP_JAL" << std::endl;
            this->OP_JAL(instruction);
            break;
        case 0b001100:
            std::cout << "OP_ANDI" << std::endl;
            this->OP_ANDI(instruction);
            break;
        case 0b101000:
            std::cout << "OP_SB" << std::endl;
            this->OP_SB(instruction);
            break;
        case 0b100000:
            std::cout << "OP_LB" << std::endl;
            this->OP_LB(instruction);
            break;
        case 0b000100:
            std::cout << "OP_BEQ" << std::endl;
            this->OP_BEQ(instruction);
            break;
        case 0b010000:
            std::cout << "OP_COP0" << std::endl; // this one is for the coprocessor 0 which handles its own opcodes
            this->OP_COP0(instruction);
            break;
        default:
            std::cout << "Unhandled instruction" << std::endl;
            throw std::exception();
    }
}

uint32_t Cpu::getRegister(const RegisterIndex &t) {
    return this->regs[t.index];
}

void Cpu::setRegister(const RegisterIndex &t, const uint32_t &v) {
    this->out_regs[0] = 0; // r0 is always zero
    std::cout << "Loading (in big endian) " << std::dec << v << " into register " << t.index << std::endl;
    this->out_regs[t.index] = v;
}

// load upper immediate opcode:
// load value 'immediate' into upper 16 bits of target
void Cpu::OP_LUI(const Instruction& instruction) {
    auto immediate = instruction.imm();
    auto t = instruction.t();

    auto value = immediate << 16u; // low 16bit set to 0

    this->setRegister(t, value);
}

// bitwise or immediate opcode:
// bitwise or of value 'immediate' with source into target
void Cpu::OP_ORI(const Instruction& instruction) {
    auto immediate = instruction.imm();
    auto t = instruction.t();
    auto s = instruction.s();

    auto value = this->getRegister(s) | immediate;

    this->setRegister(t, value);
}

// store halfword
void Cpu::OP_SH(const Instruction &instruction) {

    if ((this->sr & 0x10000u) != 0u) {
        // cache is isolated, ignore writing
        std::cout << "STUB:ignoring_store_while_cache_is_isolated" << std::endl;
        return;
    }

    auto immediate = instruction.imm_se(); // SW is sign extending
    auto t = instruction.t();
    auto s = instruction.s();

    auto address = this->getRegister(s) + immediate;
    auto value = this->getRegister(t);

    this->store16(address, (uint16_t) value);
}

// store word opcode:
// store the word in target in source plus memory offset of immediate
void Cpu::OP_SW(const Instruction& instruction) {

    if ((this->sr & 0x10000u) != 0) {
        // cache is isolated, ignore writing
        std::cout << "STUB:ignoring_store_while_cache_is_isolated" << std::endl;
        return;
    }

    auto immediate = instruction.imm_se(); // SW is sign extending
    auto t = instruction.t();
    auto s = instruction.s();

    auto address = this->getRegister(s) + immediate;
    auto value = this->getRegister(t);

    this->store32(address, value);
}

// load word opcode:
void Cpu::OP_LW(const Instruction &instruction) {

    if ((this->sr & 0x10000u) != 0) {
        // cache is isolated, ignore load
        std::cout << "STUB:ignoring_load_while_cache_is_isolated" << std::endl;
        return;
    }

    auto immediate = instruction.imm_se();
    auto t = instruction.t();
    auto s = instruction.s();

    auto address = this->getRegister(s) + immediate;

    auto value = this->load32(address);

    // simulate loading delay by putting into laod registers
    this->load = {t, value};
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

// check if theres an overflow happening on adding x and y
bool addOverflow(uint32_t x, uint32_t y, uint32_t &res)
{
    uint32_t temp = x + y;
    if(x>0 && y>0 && temp<0)
        return true;
    if(x<0 && y<0 && temp>0)
        return true;

    res = x + y;
    return false;
}

// add immediate
// we simply add immediate to source, save in target and throw an exception on overflow
void Cpu::OP_ADDI(const Instruction& instruction) {
    auto immediate = (int32) instruction.imm_se();
    auto t = instruction.t();
    auto s = (int32) instruction.s().index;

    uint32_t value;
    if (addOverflow(s, immediate, value)) {
        std::cout << "ADDI_overflow" << std::endl;
        throw std::exception();
    }
    this->setRegister(t, value);
}

// add unsigned
// add two registers and store in d
void Cpu::OP_ADDU(const Instruction &instruction) {
    auto t = instruction.t();
    auto s = instruction.s();
    auto d = instruction.d();

    auto value = this->getRegister(s) + this->getRegister(t);
    this->setRegister(d, value);
}


// jump
// set PC (instruction pointer) to address in immediate
void Cpu::OP_J(const Instruction& instruction) {
    auto immediate = instruction.imm_jump();

    // immediate is shifted 2 to the right, because the two LSBs of pc are always zero anyway (due to the 32bit boundary)
    this->pc = (this->pc & 0xf0000000u) | (immediate << 2u);
    std::cout << "Jumping to: " << this->pc << std::endl;
}

// jump and link
// jump and store return address in $ra ($31)
void Cpu::OP_JAL(const Instruction &instruction) {
    auto ra = this->pc;

    // store return in ra
    this->setRegister({ 31 }, ra);

    this->OP_J(instruction);
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

// set on less than unsigned
// set rd to 0 1 depending on wheter rs is less than rt
void Cpu::OP_SLTU(const Instruction& instruction) {
    auto s = instruction.s();
    auto t = instruction.t();
    auto d = instruction.d();

    auto value = (uint32_t) (this->getRegister(s) < this->getRegister(t));
    this->setRegister(d, value);
}

// branch to immediate value offset
void Cpu::branch(uint32_t offset) {
    // offset immediates are shifted 2 to the right since PC/IP addresses are aligned to 32bis
    offset = offset < 2u;

    this->pc = this->pc + offset;
    this->pc = this->pc - 4; // compensate for the pc += 4 of run_next_instruction
}

// branch (if) not equal
void Cpu::OP_BNE(const Instruction &instruction) {
    auto immediate = instruction.imm_se();
    auto s = instruction.s();
    auto t = instruction.t();

    if (this->getRegister(s) != this->getRegister(t)) {
        this->branch(immediate);
    }
}

// bitwise and immediate
void Cpu::OP_ANDI(const Instruction &instruction) {
    auto immediate = instruction.imm();
    auto t = instruction.t();
    auto s = instruction.s();

    auto value = this->getRegister(s) & immediate;

    this->setRegister(t, value);
}

// coprocessor 0
void Cpu::OP_COP0(const Instruction &instruction) {
    switch(instruction.cop_opcode()) {
        case 0b00100:
            this->OP_MTC0(instruction);
            break;
        default:
            std::cout << "Unhandled opcode for CoProcessor" << instruction.function() << std::endl;
            throw std::exception();
    }
}

// move to coprocessor0 opcode
// loads bytes into a register of cop0
void Cpu::OP_MTC0(const Instruction& instruction) {
    auto cpu_r = instruction.t();
    auto cop_r = instruction.d().index; // which register of cop0 to load into

    auto value = this->getRegister(cpu_r);

    switch (cop_r) {
        case 3:
        case 5:
        case 6:
        case 7:
        case 9:
        case 11: // breakpoint registers
            if (value != 0) {
                std::cout << "Unhandled_write_to_cop0_register:_" << std::dec << cop_r << std::endl;
                throw std::exception();
            }
        case 12: // status register
            this->sr = value;
            break;
        case 13: // cause register, for exceptions
            if (value != 0) {
                std::cout << "Unhandled_write_to_CAUSE_register:_" << std::dec << value << std::endl;
                throw std::exception();
            }
        default:
            std::cout << "STUB:Unhandled_cop0_register:_" << std::dec << cop_r << std::endl;
    }
}

// store byte
void Cpu::OP_SB(const Instruction &instruction) {

    if ((this->sr & 0x10000u) != 0u) {
        // cache is isolated, ignore writing
        std::cout << "STUB:ignoring_store_while_cache_is_isolated" << std::endl;
        return;
    }

    auto immediate = instruction.imm_se();
    auto t = instruction.t();
    auto s = instruction.s();

    auto address = this->getRegister(s) + immediate;
    auto value = this->getRegister(t);

    this->store8(address, (uint8_t) value);
}

// jump register
// set PC to value stored in a register
void Cpu::OP_JR(const Instruction &instruction) {
    auto s = instruction.s();
    std::cout << "jumping to addr in reg " << std::dec << s.index << std::endl;
    std::cout << "new PC: " << std::dec << this->getRegister(s) << std::endl;
    this->pc = this->getRegister(s);
}

uint8_t Cpu::load8(const uint32_t& address) const {
    return this->interconnect->load8(address);
}

// load byte (signed)
void Cpu::OP_LB(const Instruction &instruction) {
    auto immediate = instruction.imm_se();
    auto t = instruction.t();
    auto s = instruction.s();

    auto addr = this->getRegister(s) + immediate;

    // force sign extension by casting
    auto value = (int8_t) this->load8(addr);

    // put load in the delay slot
    this->load = { t, (uint32_t) value };
}

// branch if equal
void Cpu::OP_BEQ(const Instruction &instruction) {
    auto immediate = instruction.imm_se();
    auto s = instruction.s();
    auto t = instruction.t();

    if (this->getRegister(s) == this->getRegister(t)) {
        this->branch(immediate);
    }
}

// jump and link register
void Cpu::OP_JALR(const Instruction &instruction) {
    auto s = instruction.s();
    auto d = instruction.d();

    this->setRegister(d, this->pc);
    this->pc = this->getRegister(s);
}
