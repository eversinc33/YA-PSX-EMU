#ifndef GPU_H
#define GPU_H

#pragma once

#include "Renderer.h"
#include "CommandBuffer.h"
#include <exception>
#include "../memory/Vram.h"

class Gpu;
typedef void (Gpu::*Gpu_operation)(const uint32_t& value);

struct GPUCommand {
    CommandBuffer command;
    uint32_t words_remaining;
    Gpu_operation command_method;
    GPUCommand() 
    {
        this->command = CommandBuffer();
        this->words_remaining = 0;
    };
};

enum GP0Mode {
    Command, // Default mode: handling commands
    ImageLoad // load image to VRAM
};

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

// Display  color depth
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
        : gp0_mode(Command),
          current_command(GPUCommand()), 
          page_base_x(0), page_base_y(0),
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
          rectangle_texture_x_flip(false), rectangle_texture_y_flip(false),
          vram(Vram())
    {
        // Setup renderer
        this->renderer = new Renderer();
        // init vram
        this->vram.init();
    };
    ~Gpu() 
    {

    };

    GP0Mode gp0_mode;
    GPUCommand current_command;
    Vram vram; 
    uint32_t status_read();
    uint32_t read();
    void gp0(const uint32_t& value);
    void gp1(const uint32_t& value);
    

private:
    Renderer* renderer;

    uint8_t page_base_x; // Texture page base X coord (4 bits, 64 bytes increment)
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

    uint8_t texture_window_x_mask; // 8 px steps
    uint8_t texture_window_y_mask; // 8 px steps
    uint8_t texture_window_x_offset; // 8 px steps
    uint8_t texture_window_y_offset; // 8 px steps
    uint16_t drawing_area_left; // leftmost col of drawing area
    uint16_t drawing_area_top; // topmost col of drawing area
    uint16_t drawing_area_right; // ...
    uint16_t drawing_area_bottom; 
    uint16_t display_vram_x_start; // first col of the display are in VRAM
    uint16_t display_vram_y_start; // first line of the display are in VRAM
    uint16_t display_horiz_start; // display output horizontal start relative to HSYNC
    uint16_t display_horiz_end;
    uint16_t display_line_start; // display output first line relative to VSYNC
    uint16_t display_line_end;

    // when in image load mode
    uint16_t image_load_vram_target_x, image_load_vram_target_y;
    uint16_t image_load_vram_width, image_load_vram_height;
    uint16_t image_load_initial_x;
    bool first_texel_in_row = true;

    void gp0_nop(const uint32_t& value);
    void gp0_clear_cache(const uint32_t& value);
    void gp0_quad_mono_opaque(const uint32_t& value);
    void gp0_quad_shaded_opaque(const uint32_t& value);
    void gp0_quad_texture_blend_opaque(const uint32_t& value);
    void gp0_triangle_shaded_opaque(const uint32_t& value);
    void gp0_image_load(const uint32_t& value);
    void gp0_image_store(const uint32_t& value);
    void gp0_draw_mode(const uint32_t& value);
    void gp0_texture_window(const uint32_t& value);
    void gp0_set_drawing_area_top_left(const uint32_t& value);
    void gp0_set_drawing_area_bottom_right(const uint32_t& value);
    void gp0_drawing_offset(const uint32_t& value);
    void gp0_mask_bit_setting(const uint32_t& value);
    void gp1_reset_command_buffer(const uint32_t& value);
    void gp1_display_horizontal_range(const uint32_t& value);
    void gp1_display_vertical_range(const uint32_t& value);
    void gp1_acknowledge_irq(const uint32_t& value);
    void gp1_display_enable(const uint32_t& value);
    void gp1_display_vram_start(const uint32_t& value);
    void gp1_reset(const uint32_t& value);
    void gp1_dma_direction(const uint32_t& value);
    void gp1_display_mode(const uint32_t& value);
};

#endif