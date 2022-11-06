#include "Channel.h"
#include <exception>
#include <iostream>

uint32_t Channel::getControl() const {
    uint32_t control = 0;
    control |= (uint32_t)this->direction << 0;
    control |= (uint32_t)this->step << 1;
    control |= (uint32_t)this->chop << 8;
    control |= (uint32_t)this->sync << 9;
    control |= (uint32_t)this->chopDmaSz << 16;
    control |= (uint32_t)this->chopCpuSz << 20;
    control |= (uint32_t)this->enable << 24;
    control |= (uint32_t)this->trigger << 28;
    control |= (uint32_t)this->dummy << 29;
    return control;
}

void Channel::setControl(const uint32_t &value) {
    this->direction = (value & (uint32_t)1) != 0 ? FromRam : ToRam;
    this->step = ((value >> 1) & (uint32_t)1) != 0 ? Decrement : Increment;
    this->chop = ((value >> 8) & (uint32_t)1) != 0;
    
    switch((value >> 9) & (uint32_t)3) {
        case 0:
            this->sync = Manual; 
            break;
        case 1:
            this->sync = Request;
            break;
        case 2:
            this->sync = LinkedList;
            break;
        default:
            std::cout << "Invalid_DMA_sync_mode:0x" << std::hex << ((value >> 9) & (uint32_t)3) << std::endl;
            throw std::exception();
    }

    this->chopDmaSz = (uint8_t)((value >> 16) & (uint32_t)7);
    this->chopCpuSz = (uint8_t)((value >> 20) & (uint32_t)7);

    this->enable = ((value >> 24) & (uint32_t)1) != 0;
    this->trigger = ((value >> 28) & (uint32_t)1) != 0;

    this->dummy = (uint8_t)((value >> 29) & (uint32_t)3);
}

void Channel::setBase(const uint32_t &value) {
    // only bis 0:23 are relevant since only 16MB RAM are accessible by the DMA
    this->base = value & 0xffffff;
}

uint32_t Channel::getBlockControl() {
    return ((uint32_t)this->blockCount << 16) | this->blockSize;
}

void Channel::setBlockControl(const uint32_t &value) {
    this->blockCount = (uint16_t)(value >> 16);
    this->blockSize = (uint16_t)value;
}

bool Channel::isActive() const {
    // return true if the channel has been started
    bool triggered = true;
    // in manual sync, the CPU must set the trigger bit to start a channel
    if (this->sync == Manual) {
        triggered = this->trigger;
    } else {
        // otherwise no trigger needed
        triggered = true;
    }

    return this->enable && triggered;
}

Sync Channel::getSyncMode() const {
    return this->sync;
}

Step Channel::getStepMode() const {
    return this->step;
}