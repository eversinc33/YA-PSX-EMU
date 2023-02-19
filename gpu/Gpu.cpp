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

// Retrieve value of the status register GPUSTAT. Read-Only register
const uint32_t Gpu::status_read() 
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
            debug("ERROR:invalid_dma_direction:0x" << std::hex << dma_direction)
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
        default:
            debug("Unhandled_GP0_command_0x" << std::hex << value);
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
            debug("Unhandled_texture_depth:0x" << std::hex << ((value >> 7) & 3));
            throw std::exception();
            break;
    }

    this->dithering                = ((value >> 9) & 1) != 0;
    this->draw_to_display          = ((value >> 10) & 1) != 0;
    this->disable_textures         = ((value >> 11) & 1) != 0;
    this->rectangle_texture_x_flip = ((value >> 12) & 1) != 0;
    this->rectangle_texture_y_flip = ((value >> 13) & 1) != 0;
}