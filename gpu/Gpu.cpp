#include "Gpu.h"
#include "../util/logging.h"
#include <exception>
#include "CommandBuffer.h"

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
    // FIXME: might need to disable this if bootlocked
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
    // if a new command should be fetched
    if (this->current_command.words_remaining == 0)
    {
        uint32_t opcode = (value >> 24) & 0xff;

        switch (opcode) {
            case 0x00: 
                this->current_command.command_method = &Gpu::gp0_nop;
                this->current_command.command.len    = 1;
                break;
            case 0x01:
                this->current_command.command_method = &Gpu::gp0_clear_cache;
                this->current_command.command.len    = 1;
                break;
            case 0x28:
                this->current_command.command_method = &Gpu::gp0_quad_mono_opaque;
                this->current_command.command.len    = 5;
                break;
            case 0x2c:
                this->current_command.command_method = &Gpu::gp0_quad_texture_blend_opaque;
                this->current_command.command.len    = 9;
                break;
            case 0x30:
                this->current_command.command_method = &Gpu::gp0_triangle_shaded_opaque;
                this->current_command.command.len    = 6;
                break;
            case 0x38:
                this->current_command.command_method = &Gpu::gp0_quad_shaded_opaque;
                this->current_command.command.len    = 8;
                break;
            case 0xa0:
                this->current_command.command_method = &Gpu::gp0_image_load;
                this->current_command.command.len    = 3; // param 2 and 3 are used to calculate num of words used in transfer
                break;
            case 0xc0:
                this->current_command.command_method = &Gpu::gp0_image_store;
                this->current_command.command.len    = 3; // just as 0xA0
            case 0xe1:
                this->current_command.command_method = &Gpu::gp0_draw_mode;
                this->current_command.command.len    = 1;
                break;
            case 0xe2:
                this->current_command.command_method = &Gpu::gp0_texture_window;
                this->current_command.command.len    = 1;
                break;
            case 0xe3:
                this->current_command.command_method = &Gpu::gp0_set_drawing_area_top_left;
                this->current_command.command.len    = 1;
                break;
            case 0xe4:
                this->current_command.command_method = &Gpu::gp0_set_drawing_area_bottom_right;
                this->current_command.command.len    = 1;
                break;
            case 0xe5:
                this->current_command.command_method = &Gpu::gp0_drawing_offset;
                this->current_command.command.len    = 1;
                break;
            case 0xe6:
                this->current_command.command_method = &Gpu::gp0_mask_bit_setting;
                this->current_command.command.len    = 1;
                break;
            default:
                DEBUG("Unhandled_GP0_command_0x" << std::hex << value);
                throw std::exception();
                break;
        }
        this->current_command.words_remaining = this->current_command.command.len;
        this->current_command.command.clear();
    }

    this->current_command.words_remaining--;

    switch (this->gp0_mode) 
    {
        case GP0Mode::Command:
            this->current_command.command.push_word(value);
            if (this->current_command.words_remaining == 0)
            {
                // we have all parameters from the buffer so we can run the command
                (this->*(this->current_command.command_method))(value);
            }
            break;
        case GP0Mode::ImageLoad:
            {
                // TODO: apply mask settings 

                // copy pixel data to VRAM
                // TODO: verify order is right
                this->vram.store(
                    (uint16_t)(value >> 16), 
                    this->image_load_vram_target_x, 
                    this->image_load_vram_target_y, 
                    this->page_base_x*64,  // add texture page base to target coordinate
                    this->page_base_y*256
                );
                this->vram.store(
                    (uint16_t)(value & 0xffff), 
                    this->image_load_vram_target_x, 
                    this->image_load_vram_target_y, 
                    this->page_base_x*64, 
                    this->page_base_y*256
                );

                // move on to next pixel in next iteration
                // TODO: does the data go line by line or col by col?
                if (!first_texel_in_row && ((this->image_load_vram_target_x - this->image_load_initial_x) % this->image_load_vram_width == 0))
                {
                    // new line, start from beginning
                    this->image_load_vram_target_x -= this->image_load_vram_width;
                    this->image_load_vram_target_y += 1; 
                    first_texel_in_row = true;
                }
                else
                {
                    this->image_load_vram_target_x += 2;
                    first_texel_in_row = false;
                }
            }

            if (this->current_command.words_remaining == 0)
            {
                this->gp0_mode = GP0Mode::Command;
            }
            break;
        default:
            DEBUG("ERROR:invalid_gp0_mode");
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
        case 0x01:
            this->gp1_reset_command_buffer(value);
            break;
        case 0x02:
            this->gp1_acknowledge_irq(value);
            break;
        case 0x03:
            this->gp1_display_enable(value);
            break;
        case 0x04:
            this->gp1_dma_direction(value);
            break;
        case 0x05:
            this->gp1_display_vram_start(value);
            break;
        case 0x06:
            this->gp1_display_horizontal_range(value);
            break;
        case 0x07:
            this->gp1_display_vertical_range(value);
            break;
        case 0x08:
            this->gp1_display_mode(value);
            break; 
        default:
            DEBUG("Unhandled_GP1_command_0x" << std::hex << value);
            throw std::exception();
            break;
    }
}

// GP0(0x00): NOP
void Gpu::gp0_nop(const uint32_t& value)
{
    return;
}

// GP0(0x01): Clear texture cache
void Gpu::gp0_clear_cache(const uint32_t& value)
{
    DEBUG("STUB:gp0_clear_texture_cache");
}

// GP0(0x28): Monochrome Opaque Quadrilateral
void Gpu::gp0_quad_mono_opaque(const uint32_t& value)
{
    Position positions[4] = {
        pos_from_gp0(this->current_command.command[1]),
        pos_from_gp0(this->current_command.command[2]),
        pos_from_gp0(this->current_command.command[3]),
        pos_from_gp0(this->current_command.command[4])
    };

    // only one color, so just repeat colour
    auto color = color_from_gp0(this->current_command.command[0]);
    Color colors[4] = { 
        color, color, color, color
    };

    this->renderer->push_quad(positions, colors);
}

// GP0(0x2C): Textured Opaque Quadrilateral
void Gpu::gp0_quad_texture_blend_opaque(const uint32_t& value)
{
    // TODO: sony logo will be drawn here

    // first param: color
    auto color = color_from_gp0(this->current_command.command[0]);
    
    Position positions[4] = {
        pos_from_gp0(this->current_command.command[1]),
        pos_from_gp0(this->current_command.command[3]),
        pos_from_gp0(this->current_command.command[5]),
        pos_from_gp0(this->current_command.command[7])
    };

    // third param: CLUT+texcoord1 CLUTYYXX
    uint16_t CLUT  = (uint16_t)((this->current_command.command[4] && 0xffff0000) >> 16);
    // parse CLUT TODO refactor to own method
    uint8_t clut_x =  (uint8_t)(this->current_command.command[4] & 0x000001ff); // 0-5 -> x coord in 32byte steps
    uint8_t clut_x =  (uint8_t)((this->current_command.command[4] & 0x0000fe00) >> 8); // 6-15 -> y coord

    uint8_t texcoord_x = (uint8_t)(this->current_command.command[4] & 0x000000ff);
    uint8_t texcoord_y = (uint8_t)((this->current_command.command[4] & 0x0000ff00) >> 8);

    // fifth param: texcoord2+texpage PageYYXX
    uint16_t Page       = this->current_command.command[4] && 0xffff0000;
    uint8_t texcoord2_x = (uint8_t)(this->current_command.command[4] & 0x000000ff);
    uint8_t texcoord2_y = (uint8_t)((this->current_command.command[4] & 0x0000ff00) >> 8);
 
    // seventh param: texcoord3 0000YYXX
    uint8_t texcoord3_x = (uint8_t)(this->current_command.command[6] & 0x000000ff);
    uint8_t texcoord3_y = (uint8_t)((this->current_command.command[6] & 0x0000ff00) >> 8);

    // nineth param: texcoord4 0000YYXX
    uint8_t texcoord4_x = (uint8_t)(this->current_command.command[8] & 0x000000ff);
    uint8_t texcoord4_y = (uint8_t)((this->current_command.command[8] & 0x0000ff00) >> 8);

    // get rgba values for texture
    uint16_t texture_width = texcoord2_x - texcoord_x;
    uint16_t texture_height = texcoord2_y - texcoord_y; 
    auto buffer_size = texture_width * texture_height * 4;
    auto *texture_data = new unsigned char[buffer_size]; 
    for (uint16_t y=0; y < texture_height; ++y)
    {
        for (uint16_t x=0; x < texture_width; ++x)
        {
            // TODO pretend 4bit for now
            RGBA rgba = this->vram.get_4bit_texel(texcoord_x + x, texcoord_y + y, this->page_base_x, this->page_base_y, clut_x, clut_y);
            texture_data[y+x] = rgba.r;
            texture_data[y+x+1] = rgba.g;
            texture_data[y+x+2] = rgba.b;
            texture_data[y+x+3] = rgba.a;
        }
    }

    // push to renderer
    this->renderer->push_quad_textured(positions, texture_data, buffer_size);
    delete texture_data;
}

// GP0(0x30): Shaded Opaque Triangle
void Gpu::gp0_triangle_shaded_opaque(const uint32_t& value)
{
    Position positions[3] = {
        pos_from_gp0(this->current_command.command[1]),
        pos_from_gp0(this->current_command.command[3]),
        pos_from_gp0(this->current_command.command[5])
    };

    Color colors[3] = {
        color_from_gp0(this->current_command.command[0]),
        color_from_gp0(this->current_command.command[2]),
        color_from_gp0(this->current_command.command[4])
    };

    DEBUG("Drawing_triangle_shaded_opaque");
    this->renderer->push_triangle(positions, colors);
}

// GP0(0x38): Shaded Opaque Quadrilateral
void Gpu::gp0_quad_shaded_opaque(const uint32_t& value)
{
    Position positions[4] = {
        pos_from_gp0(this->current_command.command[1]),
        pos_from_gp0(this->current_command.command[3]),
        pos_from_gp0(this->current_command.command[5]),
        pos_from_gp0(this->current_command.command[7])
    };

    Color colors[4] = { 
        color_from_gp0(this->current_command.command[0]),
        color_from_gp0(this->current_command.command[2]),
        color_from_gp0(this->current_command.command[4]),
        color_from_gp0(this->current_command.command[6])
    };

    this->renderer->push_quad(positions, colors);
}

// GP0(0xA0): Image load (from CPU to VRAM)
void Gpu::gp0_image_load(const uint32_t& value)
{
    // param 1: coords where image will be put in vram -> 0xYYYYXXXX
    uint32_t target_vram_coords = this->current_command.command[1];
    this->image_load_vram_target_x = target_vram_coords & 0xffff;
    this->image_load_vram_target_y = target_vram_coords >> 16;

    // hacky member vars FIXME
    this->image_load_initial_x = this->image_load_vram_target_x;
    this->first_texel_in_row = true;

    // param 2: image resolution:
    uint32_t image_resolution = this->current_command.command[2];
    this->image_load_vram_width = image_resolution & 0xffff;
    this->image_load_vram_height = image_resolution >> 16;
    uint32_t image_size = this->image_load_vram_width * this->image_load_vram_height; // imgsize in 16bit pixels
    // odd number of pixels then round up, since we transfer 32 bits
    if (image_size % 2 != 0) 
    {
        image_size++;
    }

    // store num of words expected for this image
    this->current_command.words_remaining = image_size / 2;

    // put G0 to image load mode
    this->gp0_mode = GP0Mode::ImageLoad;

    DEBUG("Loading image with size:" << this->image_load_vram_width << "," << this->image_load_vram_height << " to " << this->image_load_vram_target_x << "," << this->image_load_vram_target_y);
}

// GP0(0xC0): image store
void Gpu::gp0_image_store(const uint32_t& value)
{
    uint32_t image_resolution = this->current_command.command[2];

    uint32_t width = image_resolution & 0xffff;
    uint32_t height = image_resolution >> 16;

    DEBUG("STUB:Unhandled_image_store_with_size:" << width << "," << height);
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
    int16_t drawing_x_offset = (int16_t)(x << 5) >> 5;
    int16_t drawing_y_offset = (int16_t)(y << 5) >> 5;
    this->renderer->set_drawing_offset(drawing_x_offset, drawing_y_offset);

    // TODO: FIXME: temporary hack to render screen, fix when timing is implemented
    DEBUG("RENDER----------------------------------")
    this->renderer->display();
}

// GP0(0xE6): set mask bit setting
void Gpu::gp0_mask_bit_setting(const uint32_t& value)
{
    this->force_set_mask_bit     = (value & 1) != 0;
    this->preserve_masked_pixels = (value & 2) != 0;
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

    // Also clear command buffer
    this->gp1_reset_command_buffer(0);

    // TODO: invalidate GPU cache when implemented
}

// GP1(0x01): Reset Command Buffer
void Gpu::gp1_reset_command_buffer(const uint32_t& value)
{
    this->current_command.command.clear();
    this->current_command.words_remaining = 0;
    this->gp0_mode = GP0Mode::Command;
    // TODO: clear command FIFO when implemented
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

// GP1 (0x02): Acknowledge interrupt
void Gpu::gp1_acknowledge_irq(const uint32_t& value)
{
    this->interrupted = true;
}

// GP1 (0x03): Display enable
void Gpu::gp1_display_enable(const uint32_t& value)
{
    this->display_disabled = (value & 1) !=0;
}

// GP1 (0x05): Display VRAM Start
void Gpu::gp1_display_vram_start(const uint32_t& value)
{
    this->display_vram_x_start = (uint16_t)(value & 0x3fe);
    this->display_vram_y_start = (uint16_t)((value >> 10) & 0x1ff);
}

// GP1(0x06): Display horizontal range
void Gpu::gp1_display_horizontal_range(const uint32_t& value)
{
    this->display_horiz_start = (uint16_t)(value & 0xfff);
    this->display_horiz_end   = (uint16_t)((value >> 12) & 0xfff);
}

// GP1(0x07): Display vertical range
void Gpu::gp1_display_vertical_range(const uint32_t& value)
{
    this->display_line_start = (uint16_t)(value & 0x3ff);
    this->display_line_end   = (uint16_t)((value >> 10) & 0x3ff);
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