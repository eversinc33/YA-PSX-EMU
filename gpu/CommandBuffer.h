#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#pragma once

#include <cstdint>
#include <exception>
#include "../util/logging.h"

// Buffers commands for the GP
class CommandBuffer
{
public:
    CommandBuffer() : len(0) 
    {

    };
    ~CommandBuffer() 
    {

    };

    // The longest cmd is GP0(0x3E) which takes 12 params
    uint32_t buffer[12];
    uint8_t len;

    void clear();
    void push_word(const uint32_t& word);

    uint32_t& operator [](int i)
    {
        if (i >= this->len)
        {
            DEBUG("ERROR:gp_command_buffer_out_of_bounds_access");
            throw std::exception();
        }
        return this->buffer[i];
    };

private:

};

#endif