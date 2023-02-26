#include "Vram.h"


uint16_t Vram::get_16bit_texel(const uint16_t& x, const uint16_t& y)
{
    // return texel data at x, y from VRAM
    int index = (y * VRAM_WIDTH) + x;
    // TODO: CLUT
    // TODO: return RGB object
    return this->vram[index];
}

uint16_t Vram::get_8bit_texel(const uint16_t& x, const uint16_t& y)
{
    int index = (y * VRAM_WIDTH*2) + x/2;
    // get only the wanted texel
    uint8_t texel = (this->vram[index] >> (x % 2) * 4) & 0xF; 
    // TODO: CLUT
    // TODO: return RGB object
    return texel; 
}

uint16_t Vram::get_4bit_texel(const uint16_t& x, const uint16_t& y)
{
    int index = (y * VRAM_WIDTH*4) + x/4;
    // get only the wanted texel
    uint8_t texel = (this->vram[index] >> (x % 4) * 4) & 0xF; 
    // TODO: CLUT
    // TODO: return RGB object
    return texel;
}
    
void Vram::store_16bit_texel(const uint16_t& x, const uint16_t& y, const uint16_t& data)
{
    // as 16bit -> one pixel in 16bit store
	int index = (y * VRAM_WIDTH) + x;
    this->vram[index] = data;
}

void Vram::store_8bit_texels(const uint16_t& x, const uint16_t& y, const uint16_t& data)
{
    // as 8bit -> two pixels in 16bit store
	int index = (y * VRAM_WIDTH*2) + x/2;
	this->vram[index] = (uint8_t)data;
	this->vram[index + 1] = (uint8_t)(data >> 8);
}

void Vram::store_4bit_texels(const uint16_t& x, const uint16_t& y, const uint16_t& data)
{
    // as 4 bit -> four pixels in 16bit store
	int index = (y * VRAM_WIDTH*4) + x/4;
	this->vram[index] = (uint8_t)data & 0xf;
	this->vram[index + 1] = (uint8_t)(data >> 4) & 0xf;
	this->vram[index + 2] = (uint8_t)(data >> 8) & 0xf;
	this->vram[index + 3] = (uint8_t)(data >> 12) & 0xf;
}