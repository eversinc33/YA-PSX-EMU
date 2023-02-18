#include "Interconnect.h"
#include "../memory/MemoryMap.h"
#include "../util/logging.h"

// Load 32 bit from the appropriate peripehral, by checking
// if it is in range of the memory and calculating the offset
uint32_t Interconnect::load32(const uint32_t& address) {
    if (address % 4 != 0) {
        debug("unaligned_load32_address_" << std::hex << address);
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
        debug("STUB:IRQ_control_read:_0x" << std::hex << absAddr);
        return 0; // we do not have interrupts for now so just return 0
    }
    if (DMA.contains(absAddr)) {
        uint32_t offset = (absAddr - DMA.start);
        auto major = (offset & uint32_t(0x70)) >> 4;
        auto minor = (offset & uint32_t(0xf)); 
        // Per-channel registers
        if (major <= 6) {
            auto channel = this->dma->getChannel(Port(major));
            switch (minor) {
                case 0:
                    return channel.base;
                    break;
                case 4:
                    return channel.getBlockControl();
                    break;
                case 8:
                    return channel.getControl();
                    break;
                default:
                    debug("STUB:a_unhandled_DMA_read:_0x" << std::hex << offset); // absAddr);
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
                    debug("STUB:b_unhandled_DMA_read:_0x" << std::hex << absAddr);
                    throw std::exception();
            }
        }
        else {
            debug("STUB:c_unhandled_DMA_read:_0x" << std::hex << absAddr);
            throw std::exception();
        }
    }
    if (GPU.contains(absAddr)) {
        uint32_t offset = (absAddr - GPU.start);
        switch(offset) {
            case 4:
                // bits 26,27,28 signal that the gpu is ready to receive dma blocks and do cpu access
                return 0b11100000000000000000000000000; // 0x1c000000; 
                break;
            default:
                debug("STUB:GPU_read:_0x" << std::hex << absAddr);
                return 0;
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
                debug("STUB:Unhandled_load32_from_Timer0:_0x" << absAddr);
                return 0;
                break;
            case 16:
                debug("STUB:Unhandled_load32_from_Timer1:_0x" << absAddr);
                return 0;
                break;
            case 32:
                debug("STUB:Unhandled_load32_from_Timer2:_0x" << absAddr);
                return 0;
                break;
        }
    }
    if (SPU.contains(absAddr)) {
        debug("STUB:Unhandled_load32_from_SPU");
        return 0;
    }

    debug("Unhandled_load32_from_" << absAddr);
    throw std::exception();
}

void Interconnect::store32(const uint32_t& address, const uint32_t& value) {
    if (address % 4 != 0) {
        debug("unaligned_store32_address_" << std::hex << address);
        throw std::exception();
    }

    auto absAddr = this->maskRegion(address);

    if (HARDWARE_REGISTERS.contains(absAddr)) {
        uint32_t offset = (absAddr - HARDWARE_REGISTERS.start);
        switch (offset) {
            // at offsets 0 and 4, the base address of the expansion 1 and 2 register maps are stored, these should never change
            case 0:
                if (value != 0x1f000000) {
                    debug("Bad_expansion_1_base_address:0x" << std::hex << value);
                    throw std::exception();
                }
                break;
            case 4:
                if (value != 0x1f802000) {
                    debug("Bad_expansion_2_base_address:0x" << std::hex << value);
                    throw std::exception();
                }
                break;
            default:
                debug("STUB:Unhandled_write_to_MEMCONTROL_register:0x" << std::hex << value);
        }
        return;
    }
    if (RAM_SIZE_REGISTER.contains(absAddr)) {
        debug("STUB:Unhandled_write_to_RAM_SIZE_register:0x" << std::hex << value);
        return;
    }
    if (CACHE_CONTROL.contains(absAddr)) {
        debug("STUB:Unhandled_write_to_CACHE_CONTROL_register:0x" << std::hex << value);
        return;
    }
    if (IRQ_CONTROL.contains(absAddr)) {
        debug("STUB:Unhandled_write_to_IRQ_CONTROL_register:0x" << std::hex << value);
        return;
    }
    if (DMA.contains(absAddr)) {
        uint32_t offset = (absAddr - DMA.start);
        auto major = (offset & (uint32_t)0x70) >> 4;
        auto minor = (offset & (uint32_t)0xf); 
        // Per-channel registers
        if (major <= 6) {
            Port port = Port(major);
            auto channel = this->dma->getChannel(port);
            switch (minor) {
                case 0:
                    channel.setBase(value);
                    break;  
                case 4:
                    channel.setBlockControl(value);
                    break;
                case 8:
                    channel.setControl(value);
                    break;
                default:
                    debug("STUB:Unhandled_write_to_DMA_register:0x" << std::hex << absAddr);
                    throw std::exception();
            }
            if (channel.isActive()) {
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
                    debug("STUB:Unhandled_write_to_DMA_register:0x" << std::hex << absAddr);
                    throw std::exception();
            }
        }
        else {
            debug("STUB:Unhandled_write_to_DMA_register:0x" << std::hex << absAddr);
            throw std::exception();
        }
    }
    if (GPU.contains(absAddr)) {
        debug("STUB:Unhandled_write_to_GPU_register:0x" << std::hex << absAddr);
        return;
    }
    if (TIMERS.contains(absAddr)) {
        debug("STUB:Unhandled_write_to_TIMER_register:0x" << std::hex << absAddr);
        return;
    }
    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->store32(offset, value);
    }

    debug("unhandled_store32_address_" << std::hex << absAddr);
    throw std::exception();
}

void Interconnect::store8(const uint32_t &address, const uint8_t &value) {
    auto absAddr = this->maskRegion(address);

    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->store8(offset, value);
    }
    if (EXPANSION_1.contains(absAddr)) {
        debug("STUB:Unhandled_write_to_EXPANSION_1_register:0x" << std::hex << value);
        return;
    }
    if (EXPANSION_2.contains(absAddr)) {
        debug("STUB:Unhandled_write_to_EXPANSION_2_register:0x" << std::hex << value);
        return;
    }

    debug("unhandled_store8_address_" << std::hex << absAddr);
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

    debug("Unhandled_load8_from_" << absAddr);
    throw std::exception();
}

uint16_t Interconnect::load16(uint32_t address) {
    auto absAddr = this->maskRegion(address);

    if (SPU.contains(absAddr)) {
        debug("STUB:Unhandled_read_from_SPU_register:_0x" << std::hex << absAddr);
        return 0;
    }
    if (IRQ_CONTROL.contains(absAddr)) {
        debug("STUB:Unhandled_read_from_IRQ_CONTROL_register:0x" << std::hex << absAddr);
        return 0;
    }
    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->load16(offset);
    }

    debug("unhandled_load16_address_" << std::hex << absAddr);
    throw std::exception();
}

void Interconnect::store16(const uint32_t &address, const uint16_t &value) {
    if (address % 2 != 0) {
        debug("unaligned_store16_address_" << std::hex << address);
        throw std::exception();
    }

    auto absAddr = this->maskRegion(address);

    if (SPU.contains(absAddr)) {
        debug("STUB:Unhandled_write_to_SPU_register:0x" << std::hex << value);
        return;
    }
    if (TIMERS.contains(absAddr)) {
        debug("STUB:Unhandled_write_to_TIMER_register:0x" << std::hex << value);
        return;
    }
    if (IRQ_CONTROL.contains(absAddr)) {
        debug("STUB:Unhandled_write_to_IRQ_CONTROL_register:0x" << std::hex << value);
        return;
    }
    if (this->ram->range.contains(absAddr)) {
        uint32_t offset = (absAddr - this->ram->range.start);
        return this->ram->store16(offset, value);
    }

    debug("unhandled_store16_address_" << std::hex << absAddr);
    throw std::exception();
}

uint32_t Interconnect::maskRegion(const uint32_t &address) {
    auto index = (uint8_t) (address >> 29u);
    return address & REGION_MASK[index];
}

void Interconnect::doDma(const Port &port) {
    // DMA Transfer to/from RAM
    // for now, ignoring chopping/priority handling 
    switch(this->dma->getChannel(port).getSyncMode()) {
        case LinkedList:
            this->doDmaLinkedList(port);
            break;
        default:
            this->doDmaBlock(port);
            break;
    }
}

void Interconnect::doDmaBlock(const Port &port) {
    debug("Starting DMA block mode");

    auto channel = this->dma->getChannel(port);
    auto increment = (channel.getStepMode() == Increment) ? 4 : -4;
    auto addr = channel.base;

    // transfer size in words
    auto transferSize = channel.getTransferSize();

    while (transferSize > 0) {
        // mask addr to ignore the two LSBs
        auto currentAddr = addr & 0x1ffffc;
        uint32_t srcWord;

        switch(channel.direction) {
            case FromRam:
                srcWord = this->ram->load32(currentAddr);
                switch(port) {
                    case Gpu_port:
                        debug("STUB:Gpu_data_0x" << std::hex << srcWord);
                        break;
                    default:
                        debug("Unhandled_FROM_RAM_dma_direction");
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
                        debug("!Unhandled_DMA_port:" << (uint8_t)port);
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

    channel.done();
}

// Emulate DMA transfer for linked list synchronization mode
void Interconnect::doDmaLinkedList(const Port &port) {
    debug("Starting DMA linked list ");

    auto channel = this->dma->getChannel(port);

    auto addr = channel.base & 0x1ffffc;

    if (channel.direction == ToRam) {
        debug("Invalid_direction_for_linked_list_mode");
        throw std::exception();
    }

    if (port != Gpu_port) {
        debug("Linked_list_mode_attempted_on_non_gpu_port:0x" << std::hex << port);
        throw std::exception();
    }

    // parse linked list
    while (true) {
        // Each entry starts with a header word. The high bytes contains the number of words in the packet
        // that follow the header.
        auto header = this->ram->load32(addr);
        auto remSz = header >> 24;

        // process words following the header
        while (remSz > 0) {
            addr = (addr + 4) & 0x1ffffc;

            auto command = this->ram->load32(addr);

            // TODO: implement gpu commands
            debug("STUB:GPU_command:0x" << std::hex << command);
            remSz -= 1;
        }

        // check for end of mark
        if ((header & 0x800000) != 0) {
            break;
        }

        // otherwise proceed with next entry
        addr = header & 0x1ffffc;
    }

    channel.done();
}