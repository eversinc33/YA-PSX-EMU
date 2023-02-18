#include "Gpu.h"
#include <exception>

Gpu::Gpu()
{
    this->renderer = new Renderer();
    if (!this->renderer->init_sdl())
    {
        throw std::exception();
    }
}

Gpu::~Gpu()
{

}