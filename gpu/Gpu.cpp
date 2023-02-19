#include "Gpu.h"
#include "../util/logging.h"
#include <exception>

Gpu::~Gpu()
{

}

// Return the horizontal resolution from the 2 bit field hr1 and the one bit field hr1
HorizontalResolution from_fields(const uint8_t& hr1, const uint8_t& hr2) 
{
    return (hr2 & 1) | ((hr1 & 3) << 1);
}

// Retrieve value of bits 18:16 of the status register
uint32_t into_status(const uint8_t& hres)
{
    return ((uint32_t)(hres)) << 16;
}

// Retrieve value of the "read" register
uint32_t Gpu::read()
{
    // Do not implement for now
    return 0;
}

// Retrieve value of the status register GPUSTAT. Read-Only register
uint32_t Gpu::status_read() 
{
    uint32_t regval = 0;

    regval |= ((uint32_t)this->page_base_x) << 0;
    regval |= ((uint32_t)this->page_base_y) << 4;
    regval |= ((uint32_t)this->semi_transparency) << 5;
    regval |= ((uint32_t)this->texture_depth) << 7;
    regval |= ((uint32_t)this->dithering) << 9;
    regval |= ((uint32_t)this->draw_to_display) << 10;
    regval |= ((uint32_t)this->force_set_mask_bit) << 11;
    regval |= ((uint32_t)this->preserve_masked_pixels) << 12;
    regval |= ((uint32_t)this->field) << 13;
    // bit 14: not supported - seems to not be in use anyway
    regval |= ((uint32_t)this->disable_textures) << 15;
    regval |= into_status(this->hres);
    regval |= ((uint32_t)this->vres) << 19;
    regval |= ((uint32_t)this->vmode) << 20;
    regval |= ((uint32_t)this->display_depth) << 21;
    regval |= ((uint32_t)this->interlaced) << 22;
    regval |= ((uint32_t)this->display_disabled) << 23;
    regval |= ((uint32_t)this->interrupted) << 24;
    
    // for now we pretend the GPU is always read 
    // ready to receive commands
    regval |= uint32_t(1) << 26;
    // ready to send VRAM to CPU
    regval |= uint32_t(1) << 27;
    // ready to receive a DMA block
    regval |= uint32_t(1) << 28;

    regval |= ((uint32_t)this->dma_direction) << 29;

    // Bit 31 changes depending on the currently drawed line
    // depending on whether its even, odd or in the vblack
    // ignore for now
    regval |= uint32_t(0) << 31;

    // note sure about this - probably the signal checked by the DMA in when sending data in request sync mode. Follows nocash spec
    uint32_t dma_request;
    switch (this->dma_direction)
    {
        case Off:
            dma_request = 0;
            break;
        case FIFO:
            // should be 0 if FIFO is full, 1 otherwise
            dma_request = 1;
            break;
        case CpuToGp0:
            // should be the same val as status bit 28
            dma_request = (regval >> 28) & 1;
            break;
        case VRamToCPU:
            // Same as status bit 27
            dma_request = (regval >> 27) & 1;
            break;
        default:
            // Should be unreachable code
            DEBUG("ERROR:invalid_dma_direction:0x" << std::hex << dma_direction)
            break;
    }
    regval |= dma_request << 25;

    return regval;
}

// Handles write to the GP0 command register
void Gpu::gp0(const uint32_t& value)
{
    uint32_t opcode = (value >> 24) & 0xff;

    switch (opcode) {
        case 0x00: break; // NOP
        case 0xe1:
            this->gp0_draw_mode(value);
            break;
        case 0xe2:
            this->gp0_texture_window(value);
            break;
        case 0xe3:
            this->gp0_set_drawing_area_top_left(value);
            break;
        case 0xe4:
            this->gp0_set_drawing_area_bottom_right(value);
            break;
        case 0xe5:
            this->gp0_drawing_offset(value);
            break;
        default:
            DEBUG("Unhandled_GP0_command_0x" << std::hex << value);
            throw std::exception();
            break;
    }
}

// Handles write to the GP1 command register
void Gpu::gp1(const uint32_t& value)
{
    uint32_t opcode = (value >> 24) & 0xff;

    switch (opcode) {
        case 0x00: 
            this->gp1_reset(value);
            break; 
        case 0x04:
            this->gp1_dma_direction(value);
            break;
        case 0x08:
            this->gp1_display_mode(value);
            break; 
        case 0xE3:
            DEBUG("YES" << std::hex << value);
            throw std::exception();
            break;
        default:
            DEBUG("Unhandled_GP1_command_0x" << std::hex << value);
            throw std::exception();
            break;
    }
}

// GP0(0xE1) command
void Gpu::gp0_draw_mode(const uint32_t& value)
{
    this->page_base_x = (uint8_t)(value & 0xf);
    this->page_base_y = (uint8_t)((value >> 4) & 1);
    this->semi_transparency = (uint8_t)((value >> 5) & 3);

    switch((value >> 7) & 3) 
    {
        case 0:
            this->texture_depth = T4Bit;
            break;
        case 1:
            this->texture_depth = T8Bit;
            break;
        case 2:
            this->texture_depth = T15Bit;
            break;
        default:
            DEBUG("Unhandled_texture_depth:0x" << std::hex << ((value >> 7) & 3));
            throw std::exception();
            break;
    }

    this->dithering                = ((value >> 9) & 1) != 0;
    this->draw_to_display          = ((value >> 10) & 1) != 0;
    this->disable_textures         = ((value >> 11) & 1) != 0;
    this->rectangle_texture_x_flip = ((value >> 12) & 1) != 0;
    this->rectangle_texture_y_flip = ((value >> 13) & 1) != 0;
}

// GP0(0xE2): Set Texture Window
void Gpu::gp0_texture_window(const uint32_t& value)
{
    this->texture_window_x_mask   = (uint8_t)(value & 0x1f);
    this->texture_window_y_mask   = (uint8_t)((value >> 5) & 0x1f);
    this->texture_window_x_offset = (uint8_t)((value >> 10) & 0x1f);
    this->texture_window_y_offset = (uint8_t)((value >> 15) & 0x1f);
}

// GP0(0xE3): Set drawing area top left
void Gpu::gp0_set_drawing_area_top_left(const uint32_t& value)
{
    this->drawing_area_top  = (uint16_t)((value >> 10) & 0x3ff);
    this->drawing_area_left = (uint16_t)(value & 0x3ff);
    DEBUG("Current Drawing area: " << std::dec << this->drawing_area_left << " " << this->drawing_area_top << ", " << this->drawing_area_bottom << " " << (uint32_t)(this->drawing_area_right));
}

// GP0(0xE4): Set drawing area bottom right
void Gpu::gp0_set_drawing_area_bottom_right(const uint32_t& value)
{
    this->drawing_area_bottom = (uint16_t)((value >> 10) & 0x3ff);
    this->drawing_area_right  = (uint16_t)(value & 0x3ff);
    DEBUG("Current Drawing area: " << std::dec << this->drawing_area_left << " " << this->drawing_area_top << ", " << this->drawing_area_bottom << " " << (uint32_t)(this->drawing_area_right));
}

// GP0(0xE5): set drawing offset
void Gpu::gp0_drawing_offset(const uint32_t& value)
{
    uint16_t x = (uint16_t)(value & 0x7ff);
    uint16_t y = (uint16_t)((value >> 11) & 0x7ff);

    // Values are 11bit two's complement signed values so we need to shift the value to 16 bits to force sign extension
    this->drawing_x_offset = (int16_t)(x << 5) >> 5;
    this->drawing_y_offset = (int16_t)(y << 5) >> 5;
}

// GP1(0x00): soft reset
void Gpu::gp1_reset(const uint32_t& value)
{
    this->interrupted = false;
    this->page_base_x = 0;
    this->page_base_y = 0;
    this->semi_transparency = 0;
    this->texture_depth = T4Bit;
    this->texture_window_x_mask = 0;
    this->texture_window_y_mask = 0;
    this->texture_window_x_offset = 0;
    this->texture_window_y_offset = 0;
    this->dithering = false;
    this->draw_to_display = false;
    this->disable_textures = false;
    this->rectangle_texture_x_flip = false;
    this->rectangle_texture_y_flip = false;
    this->drawing_area_left = 0;
    this->drawing_area_top = 0;
    this->drawing_area_bottom = 0;
    this->drawing_area_right = 0;
    this->drawing_x_offset = 0;
    this->drawing_y_offset = 0;
    this->force_set_mask_bit = false;
    this->preserve_masked_pixels = false;
    this->dma_direction = Off;
    this->display_disabled = true;
    this->display_vram_x_start = 0;
    this->display_vram_y_start = 0;
    this->hres = from_fields(0, 0);
    this->vres = Y240Lines;
    this->vmode = NTSC;
    this->interlaced = true;
    this->display_horiz_start = 0x200;
    this->display_horiz_end = 0xC00;
    this->display_line_start = 0x10;
    this->display_line_end = 0x100;
    this->display_depth = D15Bits;

    // TODO: clear command FIFO when implemented
    // TODO: invalidate GPU cache when implemented
}

// GP1(0x80): Set display mode
void Gpu::gp1_display_mode(const uint32_t& value)
{
    uint8_t hr1 = (uint8_t)(value & 3);
    uint8_t hr2 = (uint8_t)((value >> 6) & 1);

    this->hres = from_fields(hr1, hr2);

    this->vres          = ((value & 0x4) != 0) ? Y480Lines : Y240Lines;
    this->vmode         = ((value & 0x8) != 0) ? NTSC : PAL;
    this->display_depth = ((value & 0x10) != 0) ? D24Bits : D15Bits; 

    this->interlaced = (value & 0x20) != 0;

    if ((value & 0x80) != 0) 
    {
        DEBUG("Unsupported_display_mode_0x" << std::hex << value);
        throw std::exception();
    }
}

// GP1(0x04): DMA Direction
void Gpu::gp1_dma_direction(const uint32_t& value)
{
    switch (value & 3)
    {
        case 0:
            this->dma_direction = Off;
            break;
        case 1:
            this->dma_direction = FIFO;
            break;
        case 2:
            this->dma_direction = CpuToGp0;
            break;
        case 3:
            this->dma_direction = VRamToCPU;
            break;
        default:
            DEBUG("Unhandled_DMA_direction_0x" << std::hex << value);
            throw std::exception();
            break;
    }
}