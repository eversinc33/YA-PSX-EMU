#ifndef GPU_H
#define GPU_H
#include "Renderer.h"
#include <exception>
#pragma once

// Depth of pixel values in a texture page
enum TextureDepth {
    T4Bit = 0, // 4 bits per pixel
    T8Bit = 1,
    T15Bit = 2
};

// Interlaced output splits frames in two fields
enum Field {
    Bottom = 0, // even lines
    Top = 1 // odd lines
};

typedef uint8_t HorizontalResolution;

HorizontalResolution from_fields(const uint8_t& hr1, const uint8_t& hr2);
uint32_t into_status(const uint8_t& hres);

// Video output vertical resolution
enum VerticalResolution {
    Y240Lines = 0,
    Y480Lines = 1 // only available for interlaced outptu
};

// Video Mode
enum VMode {
    NTSC = 0, // 480i60Hz
    PAL = 1   // 576i50Hz
};

// Display are color depth
enum DisplayDepth {
    D15Bits = 0, // 15 bits per pixel
    D24Bits = 1  // 24 bits per pixel
};

// Requested DMA Direction
enum DmaDirection {
    Off = 0,
    FIFO = 1,
    CpuToGp0 = 2,
    VRamToCPU = 3
};

class Gpu
{
public:
    Gpu() // We are assuming default values of 0 here
        : page_base_x(0), page_base_y(0),
          semi_transparency(0), texture_depth(T4Bit),
          dithering(false),
          draw_to_display(false),
          force_set_mask_bit(false),
          preserve_masked_pixels(false),
          field(Top),
          disable_textures(false),
          hres(from_fields(0,0)),
          vres(Y240Lines),
          vmode(NTSC),
          display_depth(D15Bits),
          interlaced(false),
          display_disabled(true),
          interrupted(false),
          dma_direction(Off),
          rectangle_texture_x_flip(false), rectangle_texture_y_flip(false)
    {
        // Setup renderer
        this->renderer = new Renderer();
        if (!this->renderer->init_sdl())
        {
            throw std::exception();
        }
    };
    ~Gpu();

    const uint32_t status_read();
    void gp0(const uint32_t& value);
    void gp0_draw_mode(const uint32_t& value);

private:
    Renderer* renderer;

    // Texture page base X coord (4 bits, 64 bytes increment)
    uint8_t page_base_x;
    uint8_t page_base_y; // 1 bit, 256 line increment
    uint8_t semi_transparency;
    TextureDepth texture_depth; // texture page color depth
    bool dithering; // enable dithering from 24 to 15 bits RGB
    bool draw_to_display; // allow drawing to display
    bool force_set_mask_bit; // force mask-bit of the pixel to 1 when writing to RAM
    bool preserve_masked_pixels; // do not draw pixels with mask bit
    Field field; // currently displayed field
    bool disable_textures;
    HorizontalResolution hres;
    VerticalResolution vres;
    VMode vmode; 
    DisplayDepth display_depth; // display depth - the gpu always draws 15 bit RGB, 24 bit output must use external assets
    bool interlaced; // output interlaced video signal instead of progressive
    bool display_disabled;
    bool interrupted;
    DmaDirection dma_direction;

    bool rectangle_texture_x_flip;
    bool rectangle_texture_y_flip;
};

#endif