#include "Vram.h"

uint16_t Vram::get_16bit_texel(const uint16_t& x, const uint16_t& y)
{
    // return texel data at x, y from VRAM
    int index = (y * VRAM_WIDTH) + x;
    return this->vram[index];
}

uint16_t Vram::get_8bit_texel(const uint16_t& x, const uint16_t& y)
{
	int index = (y * VRAM_WIDTH) + x;
    uint16_t data = this->vram[index];

    // as 8bit -> two pixels in 16bit store
	this->vram[index * 2 + 0] = (uint8_t)data;
	this->vram[index * 2 + 1] = (uint8_t)(data >> 8);
}

uint16_t Vram::get_4bit_texel(const uint16_t& x, const uint16_t& y)
{
	int index = (y * VRAM_WIDTH) + x;
    uint16_t data = this->vram[index];

    // as 4 bit -> four pixels in 16bit store
	this->vram[index * 4 + 0] = (uint8_t)data & 0xf;
	this->vram[index * 4 + 1] = (uint8_t)(data >> 4) & 0xf;
	this->vram[index * 4 + 2] = (uint8_t)(data >> 8) & 0xf;
	this->vram[index * 4 + 3] = (uint8_t)(data >> 12) & 0xf;
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
	int index = (y * VRAM_WIDTH) + x;
	this->vram[index * 2 + 0] = (uint8_t)data;
	this->vram[index * 2 + 1] = (uint8_t)(data >> 8);
}

void Vram::store_4bit_texels(const uint16_t& x, const uint16_t& y, const uint16_t& data)
{
    // as 4 bit -> four pixels in 16bit store
	int index = (y * VRAM_WIDTH) + x;
	this->vram[index * 4 + 0] = (uint8_t)data & 0xf;
	this->vram[index * 4 + 1] = (uint8_t)(data >> 4) & 0xf;
	this->vram[index * 4 + 2] = (uint8_t)(data >> 8) & 0xf;
	this->vram[index * 4 + 3] = (uint8_t)(data >> 12) & 0xf;
}