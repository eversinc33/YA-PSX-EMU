//
// Created by sven on 28.11.20.
//

#include <iostream>
#include "Bios.h"

// fetch the 32 bit little endian word at offset (offset = offset in bios memory range)
uint32_t Bios::load32(const uint32_t& offset) const {
    char b0 = this->data[offset + 0];
    char b1 = this->data[offset + 1];
    char b2 = this->data[offset + 2];
    char b3 = this->data[offset + 3];

    uint32_t little_endian = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
    return little_endian;
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
    data= (char *) malloc(fileLen+1);
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
