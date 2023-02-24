#ifndef SPU_H
#define SPU_H

#pragma once

#include <stdint.h>
#include "../memory/Range.h"
#include "../util/logging.h"
#include "VoiceChannel.h"

struct VAGSample {
    unsigned char pack_info;
    unsigned char flags;
    unsigned char packed[14];
};

class Spunit
{
public:
    const uint32_t START_ADDRESS = 0x1f801c00;
    const uint16_t SIZE = 640;

    Spunit(); 
    ~Spunit();

    Range range = Range(START_ADDRESS, SIZE);

    void store16(const uint32_t &address, const uint16_t &value);
    uint16_t load16(const uint32_t &address);
private:
    uint16_t read_from_voice_channel_register(const uint32_t& address);
    void store_to_voice_channel_register(const uint32_t& address, const uint16_t& value);

    void start_sound_play(const uint32_t& value);
    void stop_sound_play(const uint32_t& value);
    void set_spu_control_1(const uint16_t& value);
    void set_spu_control_2(const uint16_t& value);
    void set_spu_status(const uint16_t& value);
    void set_channel_mode(const uint32_t& value, const ChannelMode& mode);
    
    // registers
    VoiceChannel* channels[24] = { // 0x1f801c00 to 0x1f801d80
        new VoiceChannel(0),
        new VoiceChannel(1),
        new VoiceChannel(2),
        new VoiceChannel(3),
        new VoiceChannel(4),
        new VoiceChannel(5),
        new VoiceChannel(6),
        new VoiceChannel(7),
        new VoiceChannel(8),
        new VoiceChannel(9),
        new VoiceChannel(10),
        new VoiceChannel(11),
        new VoiceChannel(12),
        new VoiceChannel(13),
        new VoiceChannel(14),
        new VoiceChannel(15),
        new VoiceChannel(16),
        new VoiceChannel(17),
        new VoiceChannel(18),
        new VoiceChannel(19),
        new VoiceChannel(20),
        new VoiceChannel(21),
        new VoiceChannel(22),
        new VoiceChannel(23)
    }; 
    uint16_t master_volume_left  = 0; // 0x1f801d80
    uint16_t master_volume_right = 0; // 0x1f801d82
    uint16_t reverb_depth_left   = 0; // 0x1f801d84
    uint16_t reverb_depth_right  = 0; // 0x1f801d86
    // 0x1f801d8c (2 x 16bit registers, stop sound play, Write only
    uint16_t spu_mem_addr        = 0; // 0x1f801da6
    uint16_t data_to_spu         = 0; // 0x1f801da8
    uint16_t spu_control_1       = 0; // 0x1f801daa
    uint16_t spu_control_2       = 0; // 0x1f801dac
    uint16_t spu_status          = 0; // 0x1f801dae
    uint16_t cd_vol_left         = 0; // 0x1f801db0
    uint16_t cd_vol_right        = 0; // 0x1f801db2
    uint16_t ext_vol_left        = 0; // 0x1f801db4
    uint16_t ext_vol_right       = 0; // 0x1f801db6
};

#endif
