#include "Dma.h"
#include "../util/logging.h"

void Dma::setControl(const uint32_t &value)  {
    this->control = value;
}

// return status of dma interrupt
bool Dma::irq() const {
    auto channelIrq = this->channelIrqFlags & this->channelIrqEnable;

    return this->forceIrq || (this->irqEnable && channelIrq != 0);
}

// retrieve value of the interrupt register
uint32_t Dma::getInterrupt() {
    auto v = 0u;
    v |= (uint32_t) this->irqDummy;
    v |= ((uint32_t) this->forceIrq) << 15u;
    v |= ((uint32_t) this->channelIrqEnable) << 16u;
    v |= ((uint32_t) this->irqEnable) << 23u;
    v |= ((uint32_t) this->channelIrqFlags) << 24u;
    v |= ((uint32_t) this->irq()) << 31u;
    return v;
}

void Dma::setInterrupt(uint32_t val) {
    this->irqDummy = (uint8_t) (val & 0x3fu);
    this->forceIrq = (val >> 15u) & 1u;
    this->channelIrqEnable = (uint8_t) (val >> 16u);
    this->forceIrq = (val >> 23u) & 1u;

    // writing 1 to a flag resets it
    auto ack = (uint8_t) ((val >> 24u) & 0x3fu);
    this->channelIrqFlags &= ~ack;
}

Channel Dma::getChannel(const Port &port) {
    return this->channels[port];
}