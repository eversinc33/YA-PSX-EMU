#include "Vram.h"


RGBA Vram::get_16bit_texel(const uint16_t& x, const uint16_t& y)
{
    // return texel data at x, y from VRAM
    uint8_t index = (y * VRAM_WIDTH) + x;
    uint16_t texel_data = this->vram[index];

    // no CLUT used in 16 bit texels, parse data directly
    RGBA texel = {
        .r = (uint8_t)(texel_data & 0x1F),
        .g = (uint8_t)((texel_data >> 5) & 0x1f),
        .b = (uint8_t)((texel_data >> 10) & 0x1f),
        .a = (uint8_t)((texel_data >> 15) & 0x1)
    };
    
    return texel;
}

uint16_t Vram::get_8bit_texel(const uint16_t& x, const uint16_t& y)
{
    int index = (y * VRAM_WIDTH) + x/2;
    // get only the wanted texel
    uint8_t texel = (this->vram[index] >> (x % 2) * 4) & 0xF; 
    // TODO: CLUT
    // TODO: return RGB object
    return texel; 
}

uint16_t Vram::get_4bit_texel(const uint16_t& x, const uint16_t& y)
{
    int index = (y * VRAM_WIDTH) + x/4;
    // get only the wanted texel
    uint8_t texel = (this->vram[index] >> (x % 4) * 4) & 0xF; 
    // TODO: CLUT
    // TODO: return RGB object
    return texel;
}
    
void Vram::store(const uint16_t& x, const uint16_t& y, const uint16_t& data)
{
	int index = (y * VRAM_WIDTH) + x;
    this->vram[index] = data;
}