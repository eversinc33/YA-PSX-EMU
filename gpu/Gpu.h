#ifndef GPU_H
#define GPU_H
#include "Renderer.h"

#pragma once

class Gpu
{
public:
    Gpu();
    ~Gpu();

private:
    Renderer* renderer;

};

#endif