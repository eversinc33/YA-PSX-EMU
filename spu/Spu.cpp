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

uint16_t Spunit::load16(const uint32_t &address)
{
    switch(address) 
    {
        case 0x1f801dae:
            return this->spu_status;
            break;
        default:
            break; 
    }    
    DEBUG("STUB:Unhandled_read_from_SPU_register:0x" << std::hex << address);
    throw std::exception();           
    return 0;
}

void Spunit::store16(const uint32_t &address, const uint16_t &value)
{
    switch(address) 
    {
        case 0x1f801d80:
            this->master_volume_left = value;
            return; break;
        case 0x1f801d82:
            this->master_volume_right = value;
            return; break;
        case 0x1f801d84:
            this->reverb_depth_left = value;
            return; break;
        case 0x1f801d86:
            this->reverb_depth_right = value;
            return; break;
        case 0x1f801d8c:
            // left word of stop sound play
            this->stop_sound_play((((uint32_t)(value)) & 0x0000ffffu) << 16u);
            return; break;
        case 0x1f801d8e:
            // right word of stop sound play
            this->stop_sound_play((((uint32_t)(value)) & 0x0000ffffu));
            return; break;
        case 0x1f801daa:
            this->set_spu_control_1(value);
            return; break;
        case 0x1f801dae:
            this->set_spu_status(value);
            return; break;
        default:
            break;
    }
    DEBUG("STUB:Unhandled_write_to_SPU_register:0x" << std::hex << value << "_at_0x" << address);
    throw std::exception();
    return;
}


uint16_t Spunit::get_voice_register(uint8_t voice)
{
    DEBUG("STUB:get_voice_register");
    return 0; 
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

void Spunit::stop_sound_play(const uint32_t& value)
{
    // TODO: parse and stop channels accordingly. Write Only 
    DEBUG("STUB:stop_sound_play");
}