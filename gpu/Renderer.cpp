#include "Renderer.h"
#include "../util/logging.h"
#include "Constants.h"

Renderer::Renderer()
{

}

bool Renderer::init_sdl()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        debug("SDL could not initialize. SDL_Error: " << SDL_GetError());
        return false;
    }
    else
    {
        this->window = SDL_CreateWindow("PSX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH_PX, SCREEN_HEIGHT_PX, SDL_WINDOW_SHOWN);
        if (this->window == NULL)
        {
            debug("Window could not be created! SDL_Error: " << SDL_GetError());
            return false;
        }
        this->screen_surface = SDL_GetWindowSurface(this->window);
    }
    return true;
}

void Renderer::poll()
{
    SDL_Event e; 
    SDL_PollEvent(&e);
    switch (e.type)
    {
        case SDL_WINDOWEVENT:
            switch (e.window.event) 
            {
                case SDL_WINDOWEVENT_CLOSE:  
                    debug("Window closed")
                    this->should_quit = true;
                    break;
                default: break;
            }
            break; 
        case SDL_QUIT:
            this->should_quit = true;
            break;              
        default: break;
    }

    SDL_FillRect(this->screen_surface, NULL, SDL_MapRGB(this->screen_surface->format, 0x00, 0x00, 0x00));
    SDL_UpdateWindowSurface(this->window);

    if (this->should_quit)
    {
        SDL_DestroyWindow(this->window);
        SDL_Quit();
    }
}

Renderer::~Renderer()
{
}