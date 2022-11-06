#include <iostream>
#include "Interconnect.h"
#include "../memory/MemoryMap.h"

// Load 32 bit from the appropriate peripehral, by checking
// if it is in range of the memory and calculating the offset
uint32_t Interconnect::load32(const uint32_t& address) {
    if (address % 4 != 0) {
        std::cout << "unaligned_load32_address_" << std::hex << address << std::endl;
        throw std::exception();
    }

    auto absAddr = this->maskRegion(address);

    if (this->bios->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->bios->range.start);
        return this->bios->load32(offset);
    }
    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->load32(offset);
    }
    if (IRQ_CONTROL.contains(absAddr)) {
        std::cout << "STUB:IRQ_control_read:_0x" << std::hex << absAddr << std::endl;
        return 0; // we do not have interrupts for now so just return 0
    }
    if (this->dma->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->dma->range.start);
        auto major = (offset & uint32_t(0x70)) >> 4;
        auto minor = (offset & uint32_t(0xf)); 
        // Per-channel registers
        if (major <= 6) {
            auto channel = this->dma->getChannel(Port(major));
            switch (minor) {
                case 0:
                    return channel.base;
                    break;
                case 8:
                    return channel.getControl();
                    break;
                default:
                    std::cout << "STUB:a_unhandled_DMA_read:_0x" << std::hex << offset << std::endl; // absAddr << std::endl;
                    throw std::exception();
            }
        }
        // Common DMA registers
        else if (major == 7) {
            switch (minor) {
                case 0:
                    return this->dma->control;
                    break;
                case 4:
                    return this->dma->getInterrupt();
                    break;
                default:
                    std::cout << "STUB:b_unhandled_DMA_read:_0x" << std::hex << absAddr << std::endl;
                    throw std::exception();
            }
        }
        else {
            std::cout << "STUB:c_unhandled_DMA_read:_0x" << std::hex << absAddr << std::endl;
            throw std::exception();
        }
    }
    if (GPU.contains(absAddr)) {
        uint32_t offset = (absAddr - GPU.start);
        switch(offset) {
            case 4:
                return 0x10000000; // bit 28 signals that the gpu is ready to receive dma blocks
            default:
                std::cout << "STUB:GPU_read:_0x" << std::hex << absAddr << std::endl;
                return 0;
        }
    }
    if (EXPANSION_1.contains(absAddr)) {
        return 0xffffffff; // no expansion connected, so all ones
    }

    std::cout << "Unhandled_load32_from_" << absAddr << std::endl;
    throw std::exception();
}

void Interconnect::store32(const uint32_t& address, const uint32_t& value) {
    if (address % 4 != 0) {
        std::cout << "unaligned_store32_address_" << std::hex << address << std::endl;
        throw std::exception();
    }

    auto absAddr = this->maskRegion(address);

    if (HARDWARE_REGISTERS.contains(absAddr)) {
        uint32_t offset = (absAddr - HARDWARE_REGISTERS.start);
        switch (offset) {
            // at offsets 0 and 4, the base address of the expansion 1 and 2 register maps are stored, these should never change
            case 0:
                if (value != 0x1f000000) {
                    std::cout << "Bad_expansion_1_base_address:0x" << std::hex << value << std::endl;
                    throw std::exception();
                }
                break;
            case 4:
                if (value != 0x1f802000) {
                    std::cout << "Bad_expansion_2_base_address:0x" << std::hex << value << std::endl;
                    throw std::exception();
                }
                break;
            default:
                std::cout << "STUB:Unhandled_write_to_MEMCONTROL_register:0x" << std::hex << value << std::endl;
        }
        return;
    }
    if (RAM_SIZE_REGISTER.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_RAM_SIZE_register:0x" << std::hex << value << std::endl;
        return;
    }
    if (CACHE_CONTROL.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_CACHE_CONTROL_register:0x" << std::hex << value << std::endl;
        return;
    }
    if (IRQ_CONTROL.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_IRQ_CONTROL_register:0x" << std::hex << value << std::endl;
        return;
    }
    if (this->dma->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->dma->range.start);
        auto major = (offset & (uint32_t)0x70) >> 4;
        auto minor = (offset & (uint32_t)0xf); 
        // Per-channel registers
        if (major <= 6) {
            auto channel = this->dma->getChannel(Port(major));
            switch (minor) {
                case 0:
                    return channel.setBase(value);
                    break;
                case 8:
                    return channel.setControl(value);
                    break;
                default:
                    std::cout << "STUB:Unhandled_write_to_DMA_register:0x" << std::hex << absAddr << std::endl;
                    throw std::exception();
            }
        }
        // Common DMA registers
        else if (major == 7) {
            switch (minor) {
                case 0:
                    return this->dma->setControl(value);
                    break;
                case 4:
                    return this->dma->setInterrupt(value);
                    break;
                default:
                    std::cout << "STUB:Unhandled_write_to_DMA_register:0x" << std::hex << absAddr << std::endl;
                    throw std::exception();
            }
        }
        else {
            std::cout << "STUB:Unhandled_write_to_DMA_register:0x" << std::hex << absAddr << std::endl;
            throw std::exception();
        }
    }
    if (GPU.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_GPU_register:0x" << std::hex << absAddr << std::endl;
        return;
    }
    if (TIMERS.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_TIMER_register:0x" << std::hex << absAddr << std::endl;
        return;
    }
    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->store32(offset, value);
    }

    std::cout << "unhandled_store32_address_" << std::hex << absAddr << std::endl;
    throw std::exception();
}

void Interconnect::store8(const uint32_t &address, const uint8_t &value) {
    auto absAddr = this->maskRegion(address);

    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->store8(offset, value);
    }
    if (EXPANSION_1.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_EXPANSION_1_register:0x" << std::hex << value << std::endl;
        return;
    }
    if (EXPANSION_2.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_EXPANSION_2_register:0x" << std::hex << value << std::endl;
        return;
    }

    std::cout << "unhandled_store8_address_" << std::hex << absAddr << std::endl;
    throw std::exception();
}

uint8_t Interconnect::load8(const uint32_t& address) {
    auto absAddr = this->maskRegion(address);

    if (this->bios->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->bios->range.start);
        return this->bios->load8(offset);
    }
    if (EXPANSION_1.contains(absAddr)) {
        return 0xff; // no expansion implemented, default returns all ones
    }
    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->load8(offset);
    }

    std::cout << "Unhandled_load8_from_" << absAddr << std::endl;
    throw std::exception();
}

uint16_t Interconnect::load16(uint32_t address) {
    auto absAddr = this->maskRegion(address);

    if (SPU.contains(absAddr)) {
        std::cout << "STUB:Unhandled_read_from_SPU_register:_0x" << std::hex << absAddr << std::endl;
        return 0;
    }
    if (IRQ_CONTROL.contains(absAddr)) {
        std::cout << "STUB:Unhandled_read_from_IRQ_CONTROL_register:0x" << std::hex << absAddr << std::endl;
        return 0;
    }
    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->load16(offset);
    }

    std::cout << "unhandled_load16_address_" << std::hex << absAddr << std::endl;
    throw std::exception();
}

void Interconnect::store16(const uint32_t &address, const uint16_t &value) {
    if (address % 2 != 0) {
        std::cout << "unaligned_store16_address_" << std::hex << address << std::endl;
        throw std::exception();
    }

    auto absAddr = this->maskRegion(address);

    if (SPU.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_SPU_register:0x" << std::hex << value << std::endl;
        return;
    }
    if (TIMERS.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_TIMER_register:0x" << std::hex << value << std::endl;
        return;
    }
    if (IRQ_CONTROL.contains(absAddr)) {
        std::cout << "STUB:Unhandled_write_to_IRQ_CONTROL_register:0x" << std::hex << value << std::endl;
        return;
    }
    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->store16(offset, value);
    }

    std::cout << "unhandled_store16_address_" << std::hex << absAddr << std::endl;
    throw std::exception();
}

uint32_t Interconnect::maskRegion(const uint32_t &address) {
    auto index = (uint8_t) (address >> 29u);
    return address & REGION_MASK[index];
}

