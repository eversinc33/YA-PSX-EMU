#include <tiff.h>
#include <iostream>
#include <bitset>
#include "Cpu.h"

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

    // check alignment
    if (address %  2 != 0) {
        return exception(LoadAddressError);
    }

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

    // check alignment
    if (address %  4 != 0) {
        return exception(LoadAddressError);
    }

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

    // check alignment
    if (address %  4 != 0) {
        return exception(LoadAddressError);
    }

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
        return this->exception(Overflow);
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
    this->branching = true;

    // immediate is shifted 2 to the right, because the two LSBs of pc are always zero anyway (due to the 32bit boundary)
    this->next_pc = (this->next_pc & 0xf0000000u) | (immediate << 2u);
}

// jump and link
// jump and store return address in $ra ($31)
void Cpu::OP_JAL(const Instruction &instruction) {
    auto ra = this->next_pc;
    this->branching = true;

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

    this->branching = true;

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
        case 0b010000:
            this->OP_RFE(instruction);
            break;
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
    this->branching = true;

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
    this->branching = true;

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
        return this->exception(Overflow);
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

// check if theres an overflow happening on substracting x and y
bool substractOverflow(uint32_t x, uint32_t y, uint32_t &res)
{
    uint32_t temp = x - y;
    if(x>0 && y<0 && temp<x)
        return true;
    if(x<0 && y>0 && temp>x)
        return true;

    res = x - y;
    return false;
}

// substract signed
void Cpu::OP_SUB(const Instruction &instruction) {
    auto t = instruction.t();
    auto s = instruction.s();
    auto d = instruction.d();

    uint32_t value;
    if (substractOverflow(((int32) this->getRegister(s)), ((int32) this->getRegister(t)), value)) {
        return this->exception(Overflow);
    }
    this->setRegister(t, value);
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

// load halfword unsigned
void Cpu::OP_LHU(const Instruction &instruction) {
    auto immediate = instruction.imm_se();
    auto t = instruction.t();
    auto s = instruction.s();

    auto addr = this->getRegister(s) + immediate;

    if (addr % 2 != 0) {
        return this->exception(LoadAddressError);
    }

    auto value = this->load16(addr);
    // put load in the delay slot
    this->load = { t, (uint32_t) value };
}

// shift left logical variable
void Cpu::OP_SLLV(const Instruction &instruction) {
    auto t = instruction.t();
    auto s = instruction.s();
    auto d = instruction.d();

    // shift amount is truncated to lower 5 bits
    auto value = this->getRegister(t) << (this->getRegister(s) & 0x1fu);

    this->setRegister(d, value);
}

// load halfword
void Cpu::OP_LH(const Instruction &instruction) {
    auto immediate = instruction.imm_se();
    auto t = instruction.t();
    auto s = instruction.s();

    auto addr = this->getRegister(s) + immediate;

    // force sign extension by casting
    auto value = (int16_t) this->load16(addr);
    // put load in the delay slot
    this->load = { t, (uint32_t) value };
}

// exclusive or
void Cpu::OP_XOR(const Instruction &instruction) {
    auto s = instruction.s();
    auto t = instruction.t();
    auto d = instruction.d();

    auto value = this->getRegister(s) ^ this->getRegister(t);
    this->setRegister(d, value);
}

// exclusive or immediate
void Cpu::OP_XORI(const Instruction &instruction) {
    auto immediate = instruction.imm();
    auto t = instruction.t();
    auto s = instruction.s();

    auto value = this->getRegister(s) ^ immediate;
    this->setRegister(t, value);
}

// break
void Cpu::OP_BREAK(const Instruction &instruction) {
    this->exception(Break);
}

// multiply (signed)
void Cpu::OP_MULT(const Instruction &instruction) {
    auto s = instruction.s();
    auto t = instruction.t();

    // sign extend by casting
    auto a = (int64_t) ((int32_t) this->getRegister(s));
    auto b = (int64_t) ((int32_t) this->getRegister(t));

    auto value = (uint64_t) (a * b);

    this->hi = (uint32_t) (value >> 32u);
    this->lo = (uint32_t) value;
}

// coprocessor 1 opcode does not exist on the playstation
void Cpu::OP_COP1(const Instruction &instruction) {
    this->exception(CoprocessorError);
}

// coprocessor 3 opcode does not exist on the playstation
void Cpu::OP_COP3(const Instruction &instruction) {
    this->exception(CoprocessorError);
}

// coprocessor 2, GTE (geometry transform engine)
void Cpu::OP_COP2(const Instruction &instruction) {
    std::cout << "STUB:unhandled_GTE_instruction:_x0" << std::hex << instruction.opcode << std::endl;
    throw std::exception();
}

// load word left (little endian only)
void Cpu::OP_LWL(const Instruction &instruction) {
    auto immediate = instruction.imm_se();
    auto t = instruction.t();
    auto s = instruction.s();

    auto addr = this->getRegister(s) + immediate;

    // this instruction bypasses load delay restrictions:
    auto curValue = this->out_regs[t.index];

    // next, load the aligned word containing the first addressed byte
    auto alignedAddr = addr & ~3u;
    auto alignedWord = this->load32(alignedAddr);

    // depneding on the address alignment, we fetch the 1-4 most significant bytes
    // and put them in the target register
    uint32_t value;
    switch(addr & 3u) {
        case 0:
            value = (curValue & 0x00ffffffu) | (alignedWord << 24u); break;
        case 1:
            value = (curValue & 0x0000ffffu) | (alignedWord << 16u); break;
        case 2:
            value = (curValue & 0x000000ffu) | (alignedWord << 8u); break;
        case 3:
            value = (curValue & 0x00000000u) | (alignedWord << 0u); break;
    }

    this->load = {t, value};
}

// load word right (little endian only)
void Cpu::OP_LWR(const Instruction &instruction) {
    auto immediate = instruction.imm_se();
    auto t = instruction.t();
    auto s = instruction.s();

    auto addr = this->getRegister(s) + immediate;

    // this instruction bypasses load delay restrictions:
    auto curValue = this->out_regs[t.index];

    // next, load the aligned word containing the first addressed byte
    auto alignedAddr = addr & ~3u;
    auto alignedWord = this->load32(alignedAddr);

    // depneding on the address alignment, we fetch the 1-4 most significant bytes
    // and put them in the target register
    uint32_t value;
    switch(addr & 3u) {
        case 1:
            value = (curValue & 0x00000000u) | (alignedWord >> 0u); break;
        case 2:
            value = (curValue & 0xff000000u) | (alignedWord >> 8u); break;
        case 0:
            value = (curValue & 0xffff0000u) | (alignedWord >> 16u); break;
        case 3:
            value = (curValue & 0xffffff00u) | (alignedWord >> 24u); break;
    }

    this->load = {t, value};
}

// store word left
void Cpu::OP_SWL(const Instruction& instruction)  {
    auto immediate = instruction.imm_se();
    auto t = instruction.t();
    auto s = instruction.s();

    auto addr = this->getRegister(s) + immediate;
    auto value = this->getRegister(t);

    auto alignedAddr = addr & ~3u;
    auto curMem = this->load32(alignedAddr);

    uint32_t mem;
    switch(addr & 3u) {
        case 0:
            mem = (curMem & 0xffffff00u) | (value >> 24u); break;
        case 1:
            mem = (curMem & 0xffff0000u) | (value >> 16u); break;
        case 2:
            mem = (curMem & 0xff000000u) | (value >> 8u); break;
        case 3:
            mem = (curMem & 0x00000000u) | (value >> 0u); break;
    }

    this->store32(alignedAddr, mem);
}

// store word right
void Cpu::OP_SWR(const Instruction& instruction)  {
    auto immediate = instruction.imm_se();
    auto t = instruction.t();
    auto s = instruction.s();

    auto addr = this->getRegister(s) + immediate;
    auto value = this->getRegister(t);

    auto alignedAddr = addr & ~3u;
    auto curMem = this->load32(alignedAddr);

    uint32_t mem;
    switch(addr & 3u) {
        case 0:
            mem = (curMem & 0x00000000u) | (value << 0u); break;
        case 1:
            mem = (curMem & 0x000000ffu) | (value << 8u); break;
        case 2:
            mem = (curMem & 0x0000ffffu) | (value << 16u); break;
        case 3:
            mem = (curMem & 0x00ffffffu) | (value << 24u); break;
    }

    this->store32(alignedAddr, mem);
}

// load word coprocessor n
void Cpu::OP_LWC0(const Instruction& instruction) {
    // not supported by c0
    this->exception(CoprocessorError);
}
void Cpu::OP_LWC1(const Instruction& instruction) {
    // not supported by c1
    this->exception(CoprocessorError);
}
void Cpu::OP_LWC2(const Instruction& instruction) {
    std::cout << "Unhandled_GTE_LWC_instruction:_0x" << std::hex << instruction.opcode << std::endl;
    throw std::exception();
}
void Cpu::OP_LWC3(const Instruction& instruction) {
    // not supported by c3
    this->exception(CoprocessorError);
}

// store word coprocessor n
void Cpu::OP_SWC0(const Instruction& instruction) {
    // not supported by c0
    this->exception(CoprocessorError);
}
void Cpu::OP_SWC1(const Instruction& instruction) {
    // not supported by c1
    this->exception(CoprocessorError);
}
void Cpu::OP_SWC2(const Instruction& instruction) {
    std::cout << "Unhandled_GTE_SWC_instruction:_0x" << std::hex << instruction.opcode << std::endl;
    throw std::exception();
}
void Cpu::OP_SWC3(const Instruction& instruction) {
    // not supported by c3
    this->exception(CoprocessorError);
}

void Cpu::OP_ILLEGAL(const Instruction &instruction) {
    std::cout << "Illegal_instruction:_0x" << std::hex << instruction.opcode << "/" << std::bitset<8>(instruction.function()) << std::endl;
    this->exception(IllegalInstruction);
}

// shift right logical variable
void Cpu::OP_SRLV(const Instruction &instruction) {
    auto t = instruction.t();
    auto s = instruction.s();
    auto d = instruction.d();

    // shit amount truncated to 5 bits
    auto value = this->getRegister(t) >> (this->getRegister(s) & 0x1fu);

    this->setRegister(d, value);
}

// shift right arithmetic variable
void Cpu::OP_SRAV(const Instruction &instruction) {
    auto t = instruction.t();
    auto s = instruction.s();
    auto d = instruction.d();

    // shit amount truncated to 5 bits
    auto value = ((int32_t) this->getRegister(t)) >> (this->getRegister(s) & 0x1fu);

    this->setRegister(d, (uint32_t) value);
}
// TODO debug after first NOR
