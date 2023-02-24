#ifndef VOICECHANNEL_H
#define VOICECHANNEL_H

#pragma once

#include <stdint.h>

enum ChannelMode {
    FM,
    NoiseGenerator,
    Reverb
};

class VoiceChannel
{
public:
    VoiceChannel(int voice_number) 
        : voice_number(voice_number)
    {
        
    };
    ~VoiceChannel()
    {

    };

    ChannelMode mode;

    // each voice has 8*16bit registers
    uint16_t volume_left;
    uint16_t volume_right;
    uint16_t frequency;
    uint16_t startaddr_sound;
    uint16_t attack_rate;
    uint16_t adsr_2;
    uint16_t adsr_volume;
    uint16_t current_repeat_addr;

    void start_play();
    void stop_play();
    
private:
    int voice_number;

    // flags: https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-voice-flags
    bool key_on  = false;
    bool key_off = false;
    bool status  = false;
};


#endif