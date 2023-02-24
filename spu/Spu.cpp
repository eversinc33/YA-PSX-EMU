#include "Spu.h"
#include "../util/logging.h"
#include "../util/bitops.h"
#include <math.h>
#include <exception>

Spunit::Spunit()
{

}

Spunit::~Spunit()
{

}

uint16_t Spunit::load16(const uint32_t &address)
{
    // voice registers
    if (address >= 0x1f801c00 && address < 0x1f801d80)
    {
        return this->read_from_voice_channel_register(address);
    }

    switch(address) 
    {
        case 0x1f801dae:
            return this->spu_status;
            break;
        case 0x1f801daa:
            return this->spu_control_1;
            break;
        case 0x1f801d88:
            return 0; // TODO write only, try to ignore for now
            break;
        case 0x1f801d8a:
            return 0; // TODO write only, try to ignore for now
            break;
        case 0x1f801d8c:
            return 0; // TODO write only, try to ignore for now
            break;
        case 0x1f801d8e:
            return 0; // TODO write only, try to ignore for now
            break;
        case 0x1f801dac:
            return this->spu_control_2;
            break;
        default:
            break; 
    }    
    DEBUG("STUB:Unhandled_read_from_SPU_register:0x" << std::hex << address);
    throw std::exception();           
    return 0;
}

uint16_t Spunit::read_from_voice_channel_register(const uint32_t& address)
{
    // read a value from a register of a voice channel

    uint32_t channel_reg_offset = (address - 0x1f801c00);

    uint32_t channel    = std::floor(channel_reg_offset / (16 * 8));
    uint32_t target_reg = std::floor((channel_reg_offset - channel*(16 * 8)) / 16);
    
    switch(target_reg) 
    {
        case 0:
            return this->channels[channel]->volume_left;
            break;
        case 1:
            return this->channels[channel]->volume_right;
            break;
        case 2:
            return this->channels[channel]->frequency;
            break;
        case 3:
            return this->channels[channel]->startaddr_sound;
            break;
        case 4:
            return this->channels[channel]->attack_rate;
            break;
        case 5:
            return this->channels[channel]->adsr_2;
            break;
        case 6:
            return this->channels[channel]->adsr_volume;
            break;
        case 7:
            return this->channels[channel]->current_repeat_addr;
            break;
        default:
            DEBUG("Invalid_target_SPU_channel_register:" << std::dec << target_reg);
            throw std::exception();
            break;
    }
}

void Spunit::store_to_voice_channel_register(const uint32_t& address, const uint16_t& value)
{
    // store a value in a register of a voice channel

    uint32_t channel_reg_offset = (address - 0x1f801c00);

    uint32_t channel    = std::floor(channel_reg_offset / (16 * 8));
    uint32_t target_reg = std::floor((channel_reg_offset - channel*(16 * 8)) / 16);
    
    // DEBUG("address: " << std::dec << address);
    // DEBUG("offset: " << std::dec << channel_reg_offset);
    // DEBUG("channel: " << std::dec << channel);
    // DEBUG("reg: " << std::dec << target_reg);

    switch(target_reg) 
    {
        case 0:
            this->channels[channel]->volume_left = value;
            break;
        case 1:
            this->channels[channel]->volume_right = value;
            break;
        case 2:
            this->channels[channel]->frequency = value;
            break;
        case 3:
            this->channels[channel]->startaddr_sound = value;
            break;
        case 4:
            this->channels[channel]->attack_rate = value;
            break;
        case 5:
            this->channels[channel]->adsr_2 = value;
            break;
        case 6:
            this->channels[channel]->adsr_volume = value;
            break;
        case 7:
            this->channels[channel]->current_repeat_addr = value;
            break;
        default:
            DEBUG("Invalid_target_SPU_channel_register:" << std::dec << target_reg);
            throw std::exception();
            break;
    }
}

void Spunit::store16(const uint32_t &address, const uint16_t &value)
{
    // voice registers
    if (address >= 0x1f801c00 && address < 0x1f801d80)
    {
        this->store_to_voice_channel_register(address, value);
        return;
    }

    // other registers
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
        case 0x1f801d88:
            this->start_sound_play((((uint32_t)(value)) & 0x0000ffffu) << 16u);
            return; break;
        case 0x1f801d8a:
            this->start_sound_play(((uint32_t)(value)) & 0x0000ffffu);
            return; break;
        case 0x1f801d90:
            // left word of set channel mode to FM
            this->set_channel_mode((((uint32_t)(value)) & 0x0000ffffu) << 16u, ChannelMode::FM);
            return; break;
        case 0x1f801d92:
            // right word of set channel mode to FM
            this->set_channel_mode((((uint32_t)(value)) & 0x0000ffffu), ChannelMode::FM);
            return; break;
        case 0x1f801d94:
            // left word of set channel mode to noise gen
            this->set_channel_mode((((uint32_t)(value)) & 0x0000ffffu) << 16u, ChannelMode::NoiseGenerator);
            return; break;
        case 0x1f801d96:
            // right word of set channel mode to noise gen
            this->set_channel_mode((((uint32_t)(value)) & 0x0000ffffu), ChannelMode::NoiseGenerator);
            return; break;
        case 0x1f801d98:
            // left word of set channel mode to reverb
            this->set_channel_mode((((uint32_t)(value)) & 0x0000ffffu) << 16u, ChannelMode::Reverb);
            return; break;
        case 0x1f801d9a:
            // right word of set channel mode to reverb
            this->set_channel_mode((((uint32_t)(value)) & 0x0000ffffu), ChannelMode::Reverb);
            return; break;
        case 0x1f801da2:
            DEBUG("STUB:write_to_SPU_start_addr_reverb_buffer")
            return; break;
        case 0x1f801da6:
            this->spu_mem_addr = value;
            return; break;
        case 0x1f801da8:
            this->data_to_spu = value;
            return; break;
        case 0x1f801daa:
            this->set_spu_control_1(value);
            return; break;
        case 0x1f801dac:
            this->set_spu_control_2(value);
            return; break;
        case 0x1f801dae:
            this->set_spu_status(value);
            return; break;
        case 0x1f801db0:
            this->cd_vol_left = value;
            return; break;
        case 0x1f801db2:
            this->cd_vol_right = value;
            return; break;
        case 0x1f801db4:
            this->ext_vol_left = value;
            return; break;
        case 0x1f801db6:
            this->ext_vol_right = value;
            return; break;
        default:
            // Reverb Registers
            if (address >= 0x1f801d84 && address <= 0x1f801dfe)
            {
                DEBUG("STUB:write_to_SPU_reverb_register");
                return; break;
            }
            break;
    }
    DEBUG("STUB:Unhandled_write_to_SPU_register:0x" << std::hex << value << "_at_0x" << address);
    throw std::exception();
    return;
}

void Spunit::set_spu_control_1(const uint16_t& value)
{
    // Set SPU control register 1
    // TODO: parse values into members
    this->spu_control_1 = value;
}

void Spunit::set_spu_control_2(const uint16_t& value)
{
    // Set SPU control register 2
    // TODO: parse values into members
    this->spu_control_1 = value;
}

void Spunit::set_spu_status(const uint16_t& value)
{
    // Set SPU status register
    this->spu_status = value;
}


void Spunit::start_sound_play(const uint32_t& value)
{    
    // parse and start channels accordingly. Write Only 
    for (int i=0; i<24; ++i)
    {
        // if bit is set, 
        if (CHECK_BIT_FROM_RIGHT(value, i)) 
        {
            this->channels[i]->start_play();
        }
    }
}

void Spunit::stop_sound_play(const uint32_t& value)
{
    // parse and stop channels accordingly. Write Only 
    for (int i=0; i<24; ++i)
    {
        // if bit is set, 
        if (CHECK_BIT_FROM_RIGHT(value, i)) 
        {
            this->channels[i]->stop_play();
        }
    }
}

void Spunit::set_channel_mode(const uint32_t& value, const ChannelMode& mode)
{
    // Get which channel out of the 24
    for (int i=0; i<24; ++i)
    {
        // if bit is set, 
        if (CHECK_BIT_FROM_RIGHT(value, i)) 
        {
            this->channels[i]->mode = mode;
        }
    }
}
