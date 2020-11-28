//
// Created by sven on 28.11.20.
//

#include <iostream>
#include "Interconnect.h"

// Load 32 bit from the appropriate peripehral, by checking
// if it is in range of the memory and calculating the offset
uint32_t Interconnect::load32(uint32_t address) {
    if (address % 4 != 0) {
        std::cout << "unaligned_load32_address_" << std::hex << address << std::endl;
        throw std::exception();
    }

    address = this->maskRegion(address);

    if (this->bios->range.contains(address)) {
        uint32_t offset = (address - this->bios->range.start);
        return this->bios->load32(offset);
    }
    else if (this->ram->range.contains(address)) {
        uint32_t offset = (address - this->ram->range.start);
        return this->ram->load32(offset);
    }

    std::cout << "No peripheral for address " << address << std::endl;
    throw std::exception();
}

void Interconnect::store32(uint32_t address, const uint32_t& value) {
    if (address % 4 != 0) {
        std::cout << "unaligned_store32_address_" << std::hex << address << std::endl;
        throw std::exception();
    }

    address = this->maskRegion(address);

    if (HARDWARE_REGISTERS.contains(address)) {
        uint32_t offset = (address - HARDWARE_REGISTERS.start);
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
    } else if (RAM_SIZE_REGISTER.contains(address)) {
        std::cout << "STUB:Unhandled_write_to_RAM_SIZE_register:0x" << std::hex << value << std::endl;
    } else if (CACHE_CONTROL.contains(address)) {
        std::cout << "STUB:Unhandled_write_to_CACHE_CONTROL_register:0x" << std::hex << value << std::endl;
    } else if (this->ram->range.contains(address)) {
        uint32_t offset = (address - this->ram->range.start);
        return this->ram->store32(offset, value);
    } else {
        std::cout << "No storage peripheral for address " << address << std::endl;
        throw std::exception();
    }
}

uint32_t Interconnect::maskRegion(const uint32_t &address) {
    auto index = (uint8_t) (address >> 29u);
    return address & REGION_MASK[index];
}
