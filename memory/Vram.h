#ifndef VRAM_H
#define VRAM_H

#pragma once

#include <stdint.h>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

// 1MB of VRAM
#define VRAM_WIDTH 1024
#define VRAM_HEIGHT 512
#define VRAM_SIZE VRAM_WIDTH*VRAM_HEIGHT

struct RGBA {
    char r;
    char g;
    char b;
    char a;
};

class Vram
{
public:
    Vram() {

    };
    ~Vram() {

    };

    RGBA get_4bit_texel(const uint16_t& x, const uint16_t& y, const uint16_t& page_x, const uint16_t& page_y, const uint16_t& clut_x, const uint16_t& clut_y);
    RGBA get_8bit_texel(const uint16_t& x, const uint16_t& y, const uint16_t& page_x, const uint16_t& page_y, const uint16_t& clut_x, const uint16_t& clut_y);
    RGBA get_16bit_texel(const uint16_t& x, const uint16_t& y, const uint16_t& page_x, const uint16_t& page_y);
    void store(const uint16_t& value, const uint16_t& x, const uint16_t& y, const uint16_t& page_x, const uint16_t& page_y);
    
private:
	uint32_t pbo16; 
	uint32_t texture16;

    uint16_t* vram;
};

#endif