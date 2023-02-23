#include "Spu.h"
#include "../util/logging.h"

Spunit::Spunit()
{

}

Spunit::~Spunit()
{

}

void Spunit::set_voice_register(uint8_t voice, uint16_t value)
{

}

uint16_t Spunit::get_voice_register(uint8_t voice)
{
    return 0; // TODO
}

void Spunit::set_spu_control_1(const uint16_t& value)
{
    // Set SPU control register
    // TODO: parse values into members
    this->spu_control_1 = value;
}

void Spunit::set_spu_status(const uint16_t& value)
{
    // Set SPU status register
    this->spu_status = value;
}

void Spunit::set_master_volume_left(const uint16_t& value)
{
    this->master_volume_left = value;
}

void Spunit::set_master_volume_right(const uint16_t& value)
{
    this->master_volume_right = value;
}

void Spunit::set_reverb_depth_left(const uint16_t& value)
{
    this->reverb_depth_left = value;
}

void Spunit::set_reverb_depth_right(const uint16_t& value)
{
    this->reverb_depth_right = value;
}

void Spunit::stop_sound_play(const uint32_t& value)
{
    // TODO: parse and stop channels accordingly. Write Only 
    DEBUG("STUB:stop_sound_play");
}