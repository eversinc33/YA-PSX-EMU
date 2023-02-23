#ifndef SPU_H
#define SPU_H

#pragma once

#include <stdint.h>
#include "../memory/Range.h"

class Voice
{
public:
    Voice()
    {
        
    };
    ~Voice()
    {

    };

    // flags: https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-voice-flags
    bool key_on  = false;
    bool key_off = false;
    bool status  = false;
};

class Spunit
{
public:
    const uint32_t START_ADDRESS = 0x1f801c00;
    const uint16_t SIZE = 640;

    Spunit(); 
    ~Spunit();

    Range range = Range(START_ADDRESS, SIZE);

    void set_voice_register(uint8_t voice, uint16_t value);
    uint16_t get_voice_register(uint8_t voice);

    // registers
    void set_master_volume_left(const uint16_t& value);
    void set_master_volume_right(const uint16_t& value);
    void set_reverb_depth_left(const uint16_t& value);
    void set_reverb_depth_right(const uint16_t& value);
    void set_spu_control_1(const uint16_t& value);
    void set_spu_status(const uint16_t& value);
    void stop_sound_play(const uint32_t& value);
    
    uint16_t get_spu_status()
    { 
        return this->spu_status; 
    };
private:
    Voice voices[24];

    // registers
    uint16_t master_volume_left  = 0; // 0x1f801d80
    uint16_t master_volume_right = 0; // 0x1f801d82
    uint16_t reverb_depth_left   = 0; // 0x1f801d84
    uint16_t reverb_depth_right  = 0; // 0x1f801d86
    // 0x1f801d8c (2 x 16bit registers, stop sound play, Write only
    uint16_t spu_control_1       = 0; // 0x1f801daa
    uint16_t spu_status          = 0; // 0x1f801dae
};

#endif