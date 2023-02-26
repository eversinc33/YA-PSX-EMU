#ifndef VRAM_H
#define VRAM_H

#pragma once

#include <stdint.h>

// 1MB of VRAM
#define VRAM_WIDTH 1024
#define VRAM_HEIGHT 512
#define VRAM_SIZE VRAM_WIDTH*VRAM_HEIGHT

class Vram
{
public:
    Vram() {

    };
    ~Vram() {

    };

    uint16_t get_4bit_texel(const uint16_t& x, const uint16_t& y);
    uint16_t get_8bit_texel(const uint16_t& x, const uint16_t& y);
    uint16_t get_16bit_texel(const uint16_t& x, const uint16_t& y);
    void store_4bit_texels(const uint16_t& x, const uint16_t& y, const uint16_t& data);
    void store_8bit_texels(const uint16_t& x, const uint16_t& y, const uint16_t& data);
    void store_16bit_texel(const uint16_t& x, const uint16_t& y, const uint16_t& data);
    
private:
    uint16_t vram[VRAM_SIZE] = { 0 };
};

#endif