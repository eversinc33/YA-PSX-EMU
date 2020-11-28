//
// Created by sven on 28.11.20.
//

#include <iostream>
#include "Bios.h"

// fetch the 32 bit little endian word at offset (offset = offset in bios memory range)
uint32_t Bios::load32(const uint32_t& offset) const {
    uint8_t b0 = this->data[offset + 0];
    uint8_t b1 = this->data[offset + 1];
    uint8_t b2 = this->data[offset + 2];
    uint8_t b3 = this->data[offset + 3];

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
