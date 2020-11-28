//
// Created by sven on 28.11.20.
//

#include <iostream>
#include "Bios.h"


uint8_t Bios::load8(const uint32_t &offset) const {
    return this->data[(unsigned char) offset];
}

// fetch the 32 bit little endian word at offset (offset = offset in bios memory range)
uint32_t Bios::load32(const uint32_t& offset) const {
    auto b0 = (uint32_t) this->data[offset + 0];
    auto b1 = (uint32_t) this->data[offset + 1];
    auto b2 = (uint32_t) this->data[offset + 2];
    auto b3 = (uint32_t) this->data[offset + 3];

    return b0 | (b1 << 8u) | (b2 << 16u) | (b3 << 24u);
}

void Bios::readBinary(const char* fname, const uint32_t& fileLen) {
    FILE *file;

    //Open file
    file = fopen(fname, "rb");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s", fname);
        return;
    }

    //Allocate memory
    data = (unsigned char *) malloc(fileLen+1);
    if (!data)
    {
        fprintf(stderr, "Memory error!");
        fclose(file);
        return;
    }

    //Read file contents into buffer
    fread(data, fileLen, 1, file);
    fclose(file);
}

