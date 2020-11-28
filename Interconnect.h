//
// Created by sven on 28.11.20.
//

#ifndef PSXEMU_INTERCONNECT_H
#define PSXEMU_INTERCONNECT_H


#include "Bios.h"

class Interconnect {
public:
    Bios* bios;
    Interconnect(Bios* bios) {
        this->bios = bios;
    };

    uint32_t load32(const uint32_t& address) const;
};


#endif //PSXEMU_INTERCONNECT_H
