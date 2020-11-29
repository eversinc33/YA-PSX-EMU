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

    // emulate branch delay slot: execute instruction, already fetch next instruction at PC (IP)
    Instruction instruction = Instruction(this->load32(this->pc));

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
            this->OP_LUI(instruction);
            break;
        case 0b001101:
            this->OP_ORI(instruction);
            break;
        case 0b101011:
            this->OP_SW(instruction);
            break;
        case 0b000000:
            // std::cout << "000000_opcode:" << std::bitset<8>(instruction.subfunction()) << std::endl;
            switch (instruction.subfunction()) {
                case 0b001001:
                    this->OP_JALR(instruction);
                    break;
                case 0b000000:
                    this->OP_SLL(instruction);
                    break;
                case 0b100101:
                    this->OP_OR(instruction);
                    break;
                case 0b100100:
                    this->OP_AND(instruction);
                    break;
                case 0b101011:
                    this->OP_SLTU(instruction);
                    break;
                case 0b100001:
                    this->OP_ADDU(instruction);
                    break;
                case 0b100000:
                    this->OP_ADD(instruction);
                    break;
                case 0b001000:
                    this->OP_JR(instruction);
                    break;
                case 0b100011:
                    this->OP_SUBU(instruction);
                    break;
                case 0b000011:
                    this->OP_SRA(instruction);
                    break;
                case 0b011010:
                    this->OP_DIV(instruction);
                    break;
                case 0b011011:
                    this->OP_DIVU(instruction);
                    break;
                case 0b010010:
                    this->OP_MFLO(instruction);
                    break;
                case 0b000010:
                    this->OP_SRL(instruction);
                    break;
                case 0b010000:
                    this->OP_MFHI(instruction);
                    break;
                case 0b101010:
                    this->OP_SLT(instruction);
                    break;
                case 0b001100:
                    this->OP_SYSCALL(instruction);
                    break;
                /*case 0b010011:
                    this->OP_MTLO(instruction);
                    break;
                case 0b010001:
                    this->OP_MTHI(instruction);
                    break;
                case 0b100111:
                    this->OP_NOR(instruction);
                    break;*/
                default:
                    std::cout << "Unhandled_000000_opcode:" << std::bitset<8>(instruction.subfunction()) << std::endl;
                    std::cout << "opcode: " << std::hex << instruction.opcode << "/" << std::bitset<8>(instruction.function()) << std::endl;
                    throw std::exception();
            }
            break;
        case 0b001001:
            this->OP_ADDIU(instruction);
            break;
        case 0b001000:
            this->OP_ADDI(instruction);
            break;
        case 0b000010:
            this->OP_J(instruction);
            break;
        case 0b000101:
            this->OP_BNE(instruction);
            break;
        case 0b000111:
            this->OP_BGTZ(instruction);
            break;
        case 0b100011:
            this->OP_LW(instruction);
            break;
        case 0b101001:
            this->OP_SH(instruction);
            break;
        case 0b000011:
            this->OP_JAL(instruction);
            break;
        case 0b001100:
            this->OP_ANDI(instruction);
            break;
        case 0b101000:
            this->OP_SB(instruction);
            break;
        case 0b100000:
            this->OP_LB(instruction);
            break;
        case 0b100100:
            this->OP_LBU(instruction);
            break;
        case 0b000100:
            this->OP_BEQ(instruction);
            break;
        case 0b000110:
            this->OP_BLEZ(instruction);
            break;
        case 0b001010:
            this->OP_SLTI(instruction);
            break;
        case 0b001011:
            this->OP_SLTIU(instruction);
            break;
        case 0b000001:
            this->OP_BXX(instruction);
            break;
        case 0b010000:
            this->OP_COP0(instruction);
            break;
        default:
            std::cout << "Unhandled instruction" << std::endl;
            std::cout << "opcode: " << std::hex << instruction.opcode << "/" << std::bitset<8>(instruction.function()) << std::endl;
            throw std::exception();
    }
}

uint32_t Cpu::getRegister(const RegisterIndex &t) {
    return this->regs[t.index];
}

void Cpu::setRegister(const RegisterIndex &t, const uint32_t &v) {
    this->out_regs[0] = 0; // r0 is always zero
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
    auto s = instruction.s();

    uint32_t value;
    if (addOverflow(((int32) this->getRegister(s)), immediate, value)) {
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
    this->next_pc = (this->next_pc & 0xf0000000u) | (immediate << 2u);
}

// jump and link
// jump and store return address in $ra ($31)
void Cpu::OP_JAL(const Instruction &instruction) {
    auto ra = this->next_pc;

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

// and
// bitwise and
void Cpu::OP_AND(const Instruction &instruction) {
    auto s = instruction.s();
    auto t = instruction.t();
    auto d = instruction.d();

    auto value = this->getRegister(s) & this->getRegister(t);
    this->setRegister(d, value);
}

// set on less than unsigned
// set rd to 0 or 1 depending on wheter rs is less than rt
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
    auto off = offset << 2u;
    this->next_pc = this->next_pc + off;
    this->next_pc = this->next_pc - 4; // compensate for the pc += 4 of run_next_instruction
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
        case 0b00000:
            this->OP_MFC0(instruction);
            break;
        /*case 0b010000:
            this->OP_RFE(instruction);
            break;*/
        default:
            std::cout << "Unhandled opcode " << std::hex << instruction.opcode << std::endl;
            std::cout << "Unhandled opcode for CoProcessor" << std::bitset<8>(instruction.cop_opcode()) << std::endl;
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
    this->next_pc= this->getRegister(s);
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

    this->setRegister(d, this->next_pc);
    this->next_pc = this->getRegister(s);
}

// move from coprocessor0
// read from a coprocessor register into target
void Cpu::OP_MFC0(const Instruction& instruction) {
    auto cpu_r = instruction.t();
    auto cop_r = instruction.d().index;

    uint32_t value;
    switch (cop_r) {
        case 12: // status register
            value = this->sr;
            break;
        case 13: // cause register, for exceptions
            value = this->cause;
            break;
        case 14: // exception PC, store pc on esception
            value = this->epc;
            break;
        default:
            std::cout << "STUB:Unhandled_read_from_cop0_register:_" << std::dec << cop_r << std::endl;
            throw std::exception();
    }

    this->load = {cpu_r, value};

}

// add , throw exception on signed overflow
void Cpu::OP_ADD(const Instruction &instruction) {
    auto t = instruction.t();
    auto s = instruction.s();
    auto d = instruction.d();

    auto s_add = (int32) this->getRegister(s);
    auto t_add = (int32) this->getRegister(t);

    uint32_t value;
    if (addOverflow(s_add, t_add, value)) {
        std::cout << "ADD_overflow" << std::endl;
        throw std::exception();
    }

    this->setRegister(d, value);
}

// branch (if) greater than zero
void Cpu::OP_BGTZ(const Instruction &instruction) {
    auto immediate = instruction.imm_se();
    auto s = instruction.s();

    auto val = (int32) this->getRegister(s); // sign cast necessary

    if (val > 0) {
        this->branch(immediate);
    }
}

// branch (if) less (or) equal zero
void Cpu::OP_BLEZ(const Instruction &instruction) {
    auto immediate = instruction.imm_se();
    auto s = instruction.s();

    auto val = (int32) this->getRegister(s); // sign cast necessary

    if (val <= 0) {
        this->branch(immediate);
    }
}

// load byte unsigned
void Cpu::OP_LBU(const Instruction &instruction) {
    auto immediate = instruction.imm_se();
    auto t = instruction.t();
    auto s = instruction.s();

    auto addr = this->getRegister(s) + immediate;

    // force sign extension by casting
    auto value = this->load8(addr);

    // put load in the delay slot
    this->load = { t, (uint32_t) value };
}

// several opcodes: BLTZ, BLTZAL, BGEZ, BGEZAL
// bits 16 to 20 define which one
void Cpu::OP_BXX(const Instruction &instruction) {
    auto immediate = instruction.imm_se();
    auto s = instruction.s();

    // if bit 16 is set, its BGEZ, otherwise BLTZ
    bool isBgez = (instruction.opcode >> 16u) & 1u;
    // if bits 20-17 are 0x80 then the return address is linked in $ra
    bool shouldLinkReturn = ((instruction.opcode >> 17u) & 0xfu) == 8;

    auto value = (int32_t) this->getRegister(s);

    // test if LTZ
    auto test = (uint32_t) (value < 0);
    // if the test we want is GEZ, we negate the comparison above by XORing
    // this saves a branch and thus speeds it up
    test = test ^ isBgez;

    if (shouldLinkReturn) {
        auto ra = this->pc;
        this->setRegister({31}, ra);
    }

    if (test != 0) {
        this->branch(immediate);
    }
}

// substract unsigned
void Cpu::OP_SUBU(const Instruction &instruction) {
    auto t = instruction.t();
    auto s = instruction.s();
    auto d = instruction.d();

    auto value = this->getRegister(s) - this->getRegister(t);
    this->setRegister(d, value);
}

// shift right arithmetic (arithmetic = signed)
void Cpu::OP_SRA(const Instruction &instruction) {
    auto immediate = instruction.imm_shift();
    auto t = instruction.t();
    auto d = instruction.d();

    // cast to signed to preserve sign bit
    auto value = ((int32_t) this->getRegister(t)) >> immediate;

    this->setRegister(d, (uint32_t) value);
}

// shift right logical (unsigned)
void Cpu::OP_SRL(const Instruction &instruction) {
    auto immediate = instruction.imm_shift();
    auto t = instruction.t();
    auto d = instruction.d();

    // cast to signed to preserve sign bit
    auto value = this->getRegister(t) >> immediate;

    this->setRegister(d, value);
}

// divide (signed)
void Cpu::OP_DIV(const Instruction &instruction) {
    auto s = instruction.s();
    auto t = instruction.t();

    auto n = (int32_t) this->getRegister(s); // numerator
    auto d = (int32_t) this->getRegister(t); // denominator -> n / d

    if (d == 0) {
        // division by zero, set bogus result
        this->hi = (uint32_t) n;
        if (n >= 0) {
            this->lo = 0xffffffff;
        } else {
            this->lo = 1;
        }
    } else if ((uint32_t) n == 0x80000000 && d == -1) {
        // result is not representable in 32 bit signed ints
        this->hi = 0;
        this->lo = 0x80000000;
    } else {
        this->hi = (uint32_t) (n % d);
        this->lo = (uint32_t) (n / d);
    }
}

// divide unsigned
void Cpu::OP_DIVU(const Instruction &instruction) {
    auto s = instruction.s();
    auto t = instruction.t();

    auto n = this->getRegister(s); // numerator
    auto d = this->getRegister(t); // denominator -> n / d

    if (d == 0) {
        // division by zero, set bogus result
        this->hi = n;
        this->lo = 0xffffffff;
    } else {
        this->hi = n % d;
        this->lo = n / d;
    }
}

// move from lo-register
void Cpu::OP_MFLO(const Instruction &instruction) {
    // TODO should stall if division not done yet
    auto d = instruction.d();
    this->setRegister(d, this->lo);
}

// set less than immediate unsigned
void Cpu::OP_SLTIU(const Instruction &instruction) {
    auto immediate = (int32_t) instruction.imm_se();
    auto s = instruction.s();
    auto t = instruction.t();

    auto value = this->getRegister(s) < immediate;
    this->setRegister(t, (uint32_t) value);
}

// set less than immediate
// set t to 1 if s less than immediate else to 0
void Cpu::OP_SLTI(const Instruction &instruction) {
    auto immediate = (int32_t) instruction.imm_se();
    auto s = instruction.s();
    auto t = instruction.t();

    auto value = ((int32_t) this->getRegister(s)) < immediate;
    this->setRegister(t, (uint32_t) value);
}

// move from hi
void Cpu::OP_MFHI(const Instruction &instruction) {
    // TODO should stall if division not done yet
    auto d = instruction.d();
    this->setRegister(d, this->hi);
}

// set on less than (signed)
void Cpu::OP_SLT(const Instruction &instruction) {
    auto s = instruction.s();
    auto t = instruction.t();
    auto d = instruction.d();

    auto value = ((int32_t) this->getRegister(s)) < ((int32_t) this->getRegister(t));
    this->setRegister(d, (uint32_t) value);
}

void Cpu::exception(Exception exception) {
    // exception handler address depends on the BEV bit
    auto handler = (this->sr & (1u << 22u)) != 0 ? 0xbfc00180 : 0x80000080;

    // shift bits 5:0 of the status register (SR) two to the left
    // by shifting these, the cpu is put into kernel mode
    auto mode = this->sr & 0x3fu;
    this->sr &= ~0x3fu;
    this->sr |= (mode << 2u) & 0x3fu;

    // update cause register with bits 6:2 (the exception code)
    this->cause = ((uint32_t) cause) << 2u;

    // save current instruction address in EPC
    this->epc = this->current_pc;

    // no branch delay in exceptions!
    this->pc = handler;
    this->next_pc = this->pc + 4;
}

void Cpu::OP_SYSCALL(const Instruction& instruction) {
    this->exception(SysCall);
}

// move to LO
void Cpu::OP_MTLO(const Instruction &instruction) {
    auto s = instruction.s();
    this->lo = this->getRegister(s);
}

// move to HI
void Cpu::OP_MTHI(const Instruction &instruction) {
    auto s = instruction.s();
    this->hi = this->getRegister(s);
}

void Cpu::OP_NOR(const Instruction &instruction) {
    auto d = instruction.d();
    auto s = instruction.s();
    auto t = instruction.t();
    auto value = ~(this->getRegister(s) | this->getRegister(t));
    this->setRegister(d, value);
}

// return from exceptions
void Cpu::OP_RFE(const Instruction &instruction) {
    // there are more instructions with the same encoding, which the playstation does not use
    // since they are virtual memory related.
    // still check for buggy code
    if ((instruction.opcode & 0x3fu) != 0b010000) {
        std::cout << "Invalid_cop0_instruction:_" << instruction.opcode << std::endl;
    }

    // restore the pre-exception mode by shifting the interrupt bits of the status register back
    auto mode = this->sr & 0x3fu;
    this->sr &= ~0x3fu;
    this->sr |= mode >> 2u;
}
