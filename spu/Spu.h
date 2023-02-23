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
    const uint16_t MEM_SIZE = 640;

    Spunit(); 
    ~Spunit();

    Range range = Range(START_ADDRESS, MEM_SIZE);

    uint16_t load16(uint32_t address);
    void store16(uint32_t offset, uint16_t value);
private:
    Voice voices[24];

    // Registers
    uint16_t voice_regs[24] = {};    // 1F801C00h..1F801D7Fh - Voice 0..23 Registers (16bit reg per voice)
    uint8_t spu_volume_control;      // F801D80h..1F801D87h - SPU Control (volume)
    uint16_t voice_flags[16] = {};   // 1F801D88h..1F801D9Fh - Voice 0..23 Flags (six 1bit flags per voice) -> 16*uint16_t = 6bit *24
    uint32_t spu_control;            // 1F801DA2h..1F801DBFh - SPU Control (memory, control, etc.) - 30 bits
    uint32_t reverb[2];              // 1F801DC0h..1F801DFFh - Reverb configuration area - 64 bits
    uint32_t voice_internal[3] = {}; // 1F801E00h..1F801E5Fh - Voice 0..23 Internal Registers - 96 bits
    uint32_t unknown[13] = {};       // 1F801E60h..1F801E7Fh - Unknown? - 1F801E80h..1F801FFFh - Unused? - 416 bits
};

#endif