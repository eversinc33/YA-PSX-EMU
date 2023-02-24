#ifndef VRAM_H
#define VRAM_H

#pragma once

#include <stdint.h>

class Vram
{
public:
    Vram();
    ~Vram();

    uint16_t vram[1024 * 512] = { 0 }; // 1 MB Vram


private:

};

#endif