//
// Created by sven on 28.11.20.
//

#include <cstdint>
#include <algorithm>
#include "Ram.h"

// fetch the 32 bit little endian word at offset (offset = offset in ram memory range)
uint32_t Ram::load32(const uint32_t& offset) const {
    uint8_t b0 = this->data[offset + 0];
    uint8_t b1 = this->data[offset + 1];
    uint8_t b2 = this->data[offset + 2];
    uint8_t b3 = this->data[offset + 3];

    return b0 | (b1 << 8u) | (b2 << 16u) | (b3 << 24u);
}

void Ram::store32(const uint32_t &offset, const uint32_t &value) {
    auto b0 = (uint8_t) value;
    auto b1 = (uint8_t) value >> 8u;
    auto b2 = (uint8_t) value >> 16u;
    auto b3 = (uint8_t) value >> 24u;

    this->data[offset + 0] = b0;
    this->data[offset + 1] = b1;
    this->data[offset + 2] = b2;
    this->data[offset + 3] = b3;
}