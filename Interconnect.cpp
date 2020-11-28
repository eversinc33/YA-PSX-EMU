//
// Created by sven on 28.11.20.
//

#include <iostream>
#include "Interconnect.h"

// Load 32 bit from the appropriate peripehral, by checking
// if it is in range of the memory and calculating the offset
uint32_t Interconnect::load32(const uint32_t &address) const {
    if (this->bios->range.contains(address)) {
        uint32_t offset = (address - this->bios->range.start);
        return this->bios->load32(offset);
    }

    std::cout << "No peripheral for address " << address << std::endl;
    throw std::exception();
}
