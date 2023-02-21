#include "CommandBuffer.h"

void CommandBuffer::push_word(const uint32_t& word)
{
    this->buffer[this->len] = word; 
    this->len++;
}

void CommandBuffer::clear() 
{
    this->len = 0;
}