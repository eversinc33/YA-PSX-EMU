#include "Interconnect.h"
#include "../memory/MemoryMap.h"
#include "../util/logging.h"

// Load 32 bit from the appropriate peripehral, by checking
// if it is in range of the memory and calculating the offset
uint32_t Interconnect::load32(const uint32_t& address) {
    if (address % 4 != 0) {
        DEBUG("unaligned_load32_address_" << std::hex << address);
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
        DEBUG("STUB:IRQ_control_read:_0x" << std::hex << absAddr);
        return 0; // we do not have interrupts for now so just return 0
    }
    if (DMA.contains(absAddr)) {
        uint32_t offset = (absAddr - DMA.start);
        auto major = (offset & uint32_t(0x70)) >> 4;
        auto minor = (offset & uint32_t(0xf)); 
        // Per-channel registers
        if (major <= 6) {
            Channel* channel = this->dma->getChannel(Port(major));
            switch (minor) {
                case 0:
                    return channel->base;
                    break;
                case 4:
                    return channel->getBlockControl();
                    break;
                case 8:
                    return channel->getControl();
                    break;
                default:
                    DEBUG("STUB:a_unhandled_DMA_read:_0x" << std::hex << offset); // absAddr);
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
                    DEBUG("STUB:b_unhandled_DMA_read:_0x" << std::hex << absAddr);
                    throw std::exception();
            }
        }
        else {
            DEBUG("STUB:c_unhandled_DMA_read:_0x" << std::hex << absAddr);
            throw std::exception();
        }
    }
    if (GPU.contains(absAddr)) {
        uint32_t offset = (absAddr - GPU.start);
        switch(offset) {
            case 0:
                return this->gpu->read();
                break;
            case 4:
                // bits 26,27,28 signal that the gpu is ready to receive dma blocks and do cpu access
                return 0b11100000000000000000000000000; // 0x1c000000; 
                break;
            default:
                DEBUG("STUB:Unhandled_GPU_read:_0x" << std::hex << offset);
                throw std::exception();
                break;
        }
    }
    if (EXPANSION_1.contains(absAddr)) {
        return 0xffffffff; // no expansion connected, so all ones
    }
    if (TIMERS.contains(absAddr)) {
        uint32_t offset = (absAddr - TIMERS.start);
        switch(offset) {
            case 0:
                // TODO: this should not be written to (yet)
                DEBUG("STUB:Unhandled_load32_from_Timer0:_0x" << absAddr);
                return 0;
                break;
            case 16:
                DEBUG("STUB:Unhandled_load32_from_Timer1:_0x" << absAddr);
                return 0;
                break;
            case 32:
                DEBUG("STUB:Unhandled_load32_from_Timer2:_0x" << absAddr);
                return 0;
                break;
        }
    }
    if (SPU.contains(absAddr)) {
        DEBUG("STUB:Unhandled_load32_from_SPU");
        return 0;
    }

    DEBUG("Unhandled_load32_from_" << absAddr);
    throw std::exception();
}

void Interconnect::store32(const uint32_t& address, const uint32_t& value) {
    if (address % 4 != 0) {
        DEBUG("unaligned_store32_address_" << std::hex << address);
        throw std::exception();
    }

    auto absAddr = this->maskRegion(address);

    if (HARDWARE_REGISTERS.contains(absAddr)) {
        uint32_t offset = (absAddr - HARDWARE_REGISTERS.start);
        switch (offset) {
            // at offsets 0 and 4, the base address of the expansion 1 and 2 register maps are stored, these should never change
            case 0:
                if (value != 0x1f000000) {
                    DEBUG("Bad_expansion_1_base_address:0x" << std::hex << value);
                    throw std::exception();
                }
                break;
            case 4:
                if (value != 0x1f802000) {
                    DEBUG("Bad_expansion_2_base_address:0x" << std::hex << value);
                    throw std::exception();
                }
                break;
            default:
                DEBUG("STUB:Unhandled_write_to_MEMCONTROL_register:0x" << std::hex << value);
        }
        return;
    }
    if (RAM_SIZE_REGISTER.contains(absAddr)) {
        DEBUG("STUB:Unhandled_write_to_RAM_SIZE_register:0x" << std::hex << value);
        return;
    }
    if (CACHE_CONTROL.contains(absAddr)) {
        DEBUG("STUB:Unhandled_write_to_CACHE_CONTROL_register:0x" << std::hex << value);
        return;
    }
    if (IRQ_CONTROL.contains(absAddr)) {
        DEBUG("STUB:Unhandled_write_to_IRQ_CONTROL_register:0x" << std::hex << value);
        return;
    }
    if (DMA.contains(absAddr)) {
        uint32_t offset = (absAddr - DMA.start);
        uint32_t major = (offset & (uint32_t)0x70) >> 4;
        uint32_t minor = (offset & (uint32_t)0xf); 
        // Per-channel registers
        if (major <= 6) {
            Port port = Port(major);
            Channel* channel = this->dma->getChannel(port);
            switch (minor) {
                case 0:
                    channel->setBase(value);
                    break;  
                case 4:
                    channel->setBlockControl(value);
                    break;
                case 8:
                    channel->setControl(value);
                    break;
                default:
                    DEBUG("STUB:Unhandled_write_to_DMA_register:0x" << std::hex << absAddr);
                    throw std::exception();
            }
            if (channel->isActive()) {
                this->doDma(port);
            }
            return;
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
                    DEBUG("STUB:Unhandled_write_to_DMA_register:0x" << std::hex << absAddr);
                    throw std::exception();
            }
        }
        else {
            DEBUG("STUB:Unhandled_write_to_DMA_register:0x" << std::hex << absAddr);
            throw std::exception();
        }
    }
    if (GPU.contains(absAddr)) {
        uint32_t offset = (absAddr - GPU.start);
        switch (offset) 
        {
            case 0:
                this->gpu->gp0(value);
                break;
            case 4:
                this->gpu->gp1(value);
                break;
            default:
                DEBUG("STUB:Unhandled_GPU_Write_to_location:0x" << std::hex << offset << "_value:0x" << std::hex << value);
                throw std::exception();
                break;
        }
        return;
    }
    if (TIMERS.contains(absAddr)) {
        DEBUG("STUB:Unhandled_write_to_TIMER_register:0x" << std::hex << absAddr);
        return;
    }
    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->store32(offset, value);
    }

    DEBUG("unhandled_store32_address_" << std::hex << absAddr);
    throw std::exception();
}

void Interconnect::store8(const uint32_t &address, const uint8_t &value) {
    auto absAddr = this->maskRegion(address);

    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->store8(offset, value);
    }
    if (EXPANSION_1.contains(absAddr)) {
        DEBUG("STUB:Unhandled_write_to_EXPANSION_1_register:0x" << std::hex << value);
        return;
    }
    if (EXPANSION_2.contains(absAddr)) {
        DEBUG("STUB:Unhandled_write_to_EXPANSION_2_register:0x" << std::hex << value);
        return;
    }
    if (CDROM_STATUS.contains(absAddr)) {
        DEBUG("STUB:Unhandled_write_to_CDROM_STATUS_register:0x" << std::hex << value);
        return;
    }

    DEBUG("unhandled_store8_address_" << std::hex << absAddr);
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

    DEBUG("Unhandled_load8_from_" << absAddr);
    throw std::exception();
}

uint16_t Interconnect::load16(uint32_t address) {
    auto absAddr = this->maskRegion(address);

    if (SPU.contains(absAddr)) {
        DEBUG("STUB:Unhandled_read_from_SPU_register:_0x" << std::hex << absAddr);
        return 0;
    }
    if (IRQ_CONTROL.contains(absAddr)) {
        DEBUG("STUB:Unhandled_read_from_IRQ_CONTROL_register:0x" << std::hex << absAddr);
        return 0;
    }
    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->load16(offset);
    }

    DEBUG("unhandled_load16_address_" << std::hex << absAddr);
    throw std::exception();
}

void Interconnect::store16(const uint32_t &address, const uint16_t &value) {
    if (address % 2 != 0) {
        DEBUG("unaligned_store16_address_" << std::hex << address);
        throw std::exception();
    }

    auto absAddr = this->maskRegion(address);

    if (SPU.contains(absAddr)) {
        DEBUG("STUB:Unhandled_write_to_SPU_register:0x" << std::hex << value);
        return;
    }
    if (TIMERS.contains(absAddr)) {
        DEBUG("STUB:Unhandled_write_to_TIMER_register:0x" << std::hex << value);
        return;
    }
    if (IRQ_CONTROL.contains(absAddr)) {
        DEBUG("STUB:Unhandled_write_to_IRQ_CONTROL_register:0x" << std::hex << value);
        return;
    }
    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->store16(offset, value);
    }

    DEBUG("unhandled_store16_address_" << std::hex << absAddr);
    throw std::exception();
}

uint32_t Interconnect::maskRegion(const uint32_t &address) {
    auto index = (uint8_t) (address >> 29u);
    return address & REGION_MASK[index];
}

void Interconnect::doDma(const Port &port) {
    // DMA Transfer to/from RAM
    // for now, ignoring chopping/priority handling 
    DEBUG("DMA FOR PORT " << port)
    switch(this->dma->getChannel(port)->getSyncMode()) {
        case LinkedList:
            this->doDmaLinkedList(port);
            break;
        default:
            this->doDmaBlock(port);
            break;
    }
}

void Interconnect::doDmaBlock(const Port &port) {
    DEBUG("Starting DMA block mode");

    Channel* channel = this->dma->getChannel(port);
    uint8_t increment = (channel->getStepMode() == Increment) ? 4 : -4;
    uint32_t addr = channel->base;

    // transfer size in words
    uint32_t transferSize = channel->getTransferSize();

    while (transferSize > 0) {
        // mask addr to ignore the two LSBs
        uint32_t currentAddr = addr & 0x1ffffc;
        uint32_t srcWord;

        switch(channel->direction) {
            case FromRam:
                srcWord = this->ram->load32(currentAddr);
                switch(port) {
                    case Gpu_port:
                        DEBUG("STUB:Gpu_data_0x" << std::hex << srcWord);
                        break;
                    default:
                        DEBUG("Unhandled_FROM_RAM_dma_direction");
                        throw std::exception();
                        break;
                }
                break;
            case ToRam:
                switch(port) {
                    case Otc:
                        // Clear ordering table
                        if (transferSize == 1) {
                            // last entry contains the end-of-table-marker
                            srcWord = 0xffffff;
                        } else {
                            // pointer to prev entry
                            srcWord = (addr - 4) & 0x1fffff;
                        }
                        break;
                    default:
                        DEBUG("!Unhandled_DMA_port:" << (uint8_t)port);
                        throw std::exception();
                        break;
                }
                // store in ram
                this->store32(currentAddr, srcWord);
                break;
        }

        addr += increment;
        transferSize -= 1;
    }

    channel->done();
}

// Emulate DMA transfer for linked list synchronization mode
void Interconnect::doDmaLinkedList(const Port &port) {
    DEBUG("Starting DMA linked list ");

    Channel* channel = this->dma->getChannel(port);

    uint32_t addr = channel->base & 0x1ffffc;

    if (channel->direction == ToRam) {
        DEBUG("Invalid_direction_for_linked_list_mode");
        throw std::exception();
    }

    if (port != Gpu_port) {
        DEBUG("Linked_list_mode_attempted_on_non_gpu_port:0x" << std::hex << port);
        throw std::exception();
    }

    // parse linked list
    while (true) {
        // Each entry starts with a header word. The high bytes contains the number of words in the packet
        // that follow the header.
        uint32_t header = this->ram->load32(addr);
        uint32_t remSz = header >> 24;

        // process words following the header
        while (remSz > 0) 
        {
            addr = (addr + 4) & 0x1ffffc;

            uint32_t command = this->ram->load32(addr);
            this->gpu->gp0(command);

            remSz -= 1;
        }

        // check for end of mark
        if ((header & 0x800000) != 0)
        { 
            break;
        }

        // otherwise proceed with next entry
        addr = header & 0x1ffffc;
    }

    channel->done();
}