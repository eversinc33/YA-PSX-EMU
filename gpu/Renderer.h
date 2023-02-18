#ifndef RENDERER_H
#define RENDERER_H

#pragma once

#include <SDL2/SDL.h> 

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void poll();
    bool init_sdl();
private:
    SDL_Window* window;
    SDL_Surface* screen_surface;
    const int width = 640;
    const int height = 480;
    bool should_quit = false; 
};

#endif