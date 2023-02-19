#include <cstdint>
#include <algorithm>
#include <bitset>
#include <iostream>
#include "Ram.h"

// fetch the 32 bit little endian word at offset (offset = offset in ram memory range)
uint32_t Ram::load32(const uint32_t& offset) const {
    auto b0 = (uint32_t) this->data[offset + 0];
    auto b1 = (uint32_t) this->data[offset + 1];
    auto b2 = (uint32_t) this->data[offset + 2];
    auto b3 = (uint32_t) this->data[offset + 3];

    return b0 | (b1 << 8u) | (b2 << 16u) | (b3 << 24u);
}

void Ram::store32(const uint32_t &offset, const uint32_t &value) {
    auto b0 = value;
    auto b1 = value >> 8u;
    auto b2 = value >> 16u;
    auto b3 = value >> 24u;

    /*DEBUG(std::bitset<8>(b0));
    DEBUG(std::bitset<8>(b1));
    DEBUG(std::bitset<8>(b2));
    DEBUG(std::bitset<8>(b3));*/

    this->data[offset + 0] = b0;
    this->data[offset + 1] = b1;
    this->data[offset + 2] = b2;
    this->data[offset + 3] = b3;
}

void Ram::store8(const uint32_t &offset, const uint8_t &value) {
    this->data[offset] = value;
}

uint8_t Ram::load8(const uint32_t &offset) {
    return this->data[offset];
}

void Ram::store16(const uint32_t &offset, const uint8_t &value) {
    auto b0 = (uint8_t) value;
    auto b1 = (uint8_t) (value >> 8u);

    this->data[offset + 0] = b0;
    this->data[offset + 1] = b1;
}

uint16_t Ram::load16(const uint32_t& offset) {
    auto b0 = (uint16_t) this->data[offset + 0];
    auto b1 = (uint16_t) this->data[offset + 1];

    return b0 | (uint16_t) (b1 << 8u);
}
