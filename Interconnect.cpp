//
// Created by sven on 28.11.20.
//

#include <iostream>
#include "Interconnect.h"

// Load 32 bit from the appropriate peripehral, by checking
// if it is in range of the memory and calculating the offset
uint32_t Interconnect::load32(const uint32_t& address) {
    if (address % 4 != 0) {
        std::cout << "unaligned_load32_address_" << std::hex << address << std::endl;
        throw std::exception();
    }

    auto absAddr = this->maskRegion(address);

    if (this->bios->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->bios->range.start);
        return this->bios->load32(offset);
    }
    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->load32(offset);
    }
    if (EXPANSION_1.contains(absAddr)) {
        return 0xffffffff; // no expansion connected, so all ones
    }

    std::cout << "No peripheral for address " << absAddr << std::endl;
    throw std::exception();
}

void Interconnect::store8(const uint32_t &address, const uint8_t &value) {
    auto absAddr = this->maskRegion(address);

    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->store8(offset, value);
    }
    if (EXPANSION_1.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_EXPANSION_1_register:0x" << std::hex << value << std::endl;
        return;
    }
    if (EXPANSION_2.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_EXPANSION_2_register:0x" << std::hex << value << std::endl;
        return;
    }

    std::cout << "unhandled_store8_address_" << std::hex << absAddr << std::endl;
    throw std::exception();
}

uint8_t Interconnect::load8(const uint32_t& address) {
    auto absAddr = this->maskRegion(address);

    if (this->bios->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->bios->range.start);
        return this->bios->load8(offset);
    }
    if (EXPANSION_1.contains(absAddr)) {
        return 0xff; // no expansion implemented, default returns all ones
    }
    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->load8(offset);
    }

    std::cout << "No peripheral for address " << absAddr << std::endl;
    throw std::exception();
}

void Interconnect::store16(const uint32_t &address, const uint16_t &value) {
    if (address % 2 != 0) {
        std::cout << "unaligned_store16_address_" << std::hex << address << std::endl;
        throw std::exception();
    }

    auto absAddr = this->maskRegion(address);

    if (SPU.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_SPU_register:0x" << std::hex << value << std::endl;
        return;
    }

    std::cout << "unhandled_store16_address_" << std::hex << absAddr << std::endl;
    throw std::exception();
}

void Interconnect::store32(const uint32_t& address, const uint32_t& value) {
    if (address % 4 != 0) {
        std::cout << "unaligned_store32_address_" << std::hex << address << std::endl;
        throw std::exception();
    }

    auto absAddr = this->maskRegion(address);

    if (HARDWARE_REGISTERS.contains(absAddr)) {
        uint32_t offset = (absAddr - HARDWARE_REGISTERS.start);
        switch (offset) {
            // at offsets 0 and 4, the base address of the expansion 1 and 2 register maps are stored, these should never change
            case 0:
                if (value != 0x1f000000) {
                    std::cout << "Bad_expansion_1_base_address:0x" << std::hex << value << std::endl;
                    throw std::exception();
                }
                break;
            case 4:
                if (value != 0x1f802000) {
                    std::cout << "Bad_expansion_2_base_address:0x" << std::hex << value << std::endl;
                    throw std::exception();
                }
                break;
            default:
                std::cout << "STUB:Unhandled_write_to_MEMCONTROL_register:0x" << std::hex << value << std::endl;
        }
    } else if (RAM_SIZE_REGISTER.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_RAM_SIZE_register:0x" << std::hex << value << std::endl;
    } else if (CACHE_CONTROL.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_CACHE_CONTROL_register:0x" << std::hex << value << std::endl;
    } else if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->store32(offset, value);
    } else {
        std::cout << "No storage peripheral for address " << absAddr << std::endl;
        throw std::exception();
    }
}

uint32_t Interconnect::maskRegion(const uint32_t &address) {
    auto index = (uint8_t) (address >> 29u);
    return address & REGION_MASK[index];
}

