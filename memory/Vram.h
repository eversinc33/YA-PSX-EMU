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

    void init()
    {
        uint32_t access = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
	
	    // 16bit VRAM pixel buffer
        glGenBuffers(1, &pbo16);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo16);
        glBufferStorage(GL_PIXEL_UNPACK_BUFFER, VRAM_SIZE, nullptr, access);	
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glGenTextures(1, &texture16);
        glBindTexture(GL_TEXTURE_2D, texture16);

        // texture wrapping and filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // allocate sapce on gpu and bind
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, VRAM_WIDTH, VRAM_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, texture16);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo16);
	    this->ptr16 = (uint16_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, VRAM_SIZE, access);
    };

    void upload() 
    {
        // Upload 16bit texture to GPU
        glBindTexture(GL_TEXTURE_2D, texture16);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo16);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, VRAM_WIDTH, VRAM_HEIGHT, GL_RED, GL_UNSIGNED_BYTE, 0);
    };

    RGBA get_4bit_texel(const uint16_t& x, const uint16_t& y, const uint16_t& page_x, const uint16_t& page_y);
    RGBA get_8bit_texel(const uint16_t& x, const uint16_t& y, const uint16_t& page_x, const uint16_t& page_y);
    RGBA get_16bit_texel(const uint16_t& x, const uint16_t& y, const uint16_t& page_x, const uint16_t& page_y);
    void store(const uint16_t& value, const uint16_t& x, const uint16_t& y, const uint16_t& page_x, const uint16_t& page_y);
    
private:
	uint32_t pbo4, pbo8, pbo16; 
	uint32_t texture4, texture8, texture16;
    uint8_t* ptr4;
    uint8_t* ptr8;
    uint16_t* ptr16;

    uint16_t vram[VRAM_SIZE] = { 0 };
    uint16_t clut_x; // TODO
    uint16_t clut_y; // TODO
};

#endif