#include "Spu.h"
#include "../util/logging.h"

Spunit::Spunit()
{

}

Spunit::~Spunit()
{

}

uint16_t Spunit::load16(uint32_t offset)
{
    if (offset < 384) // 0x00-0x180: voice registers
    {
        uint16_t target_voice = offset % 16;
        uint16_t val = this->voice_regs[target_voice];
        DEBUG("load_16_from_voice:" << std::dec << target_voice << ":" << val);
        return val;
    } 

    DEBUG("...................................." << offset);
    DEBUG("STUB:Unhandled_read_from_SPU_register_at_offset:_0x" << std::hex << offset);
    return 0;
}


void Spunit::store16(uint32_t offset, uint16_t value)
{
    if (offset < 384) // 0x00-0x180: voice registers
    {
        uint16_t target_voice = offset % 16;
        this->voice_regs[target_voice] = value;
        DEBUG("store_16_to_voice:" << std::dec << target_voice << ":" << value);
        return;
    } 

    DEBUG("STUB:Unhandled_write_to_SPU_register:0x" << std::hex << offset);
}