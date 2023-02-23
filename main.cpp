#include <cstdint>
#include "bios/Bios.h"
#include "cpu/Cpu.h"
#include "gpu/Gpu.h"
#include "spu/Spu.h"
#include "memory/Ram.h"
#include "util/logging.h"
#include <SDL2/SDL.h>

const char* BIOS_FNAME   = "./SCPH1001.BIN";
const uint32_t BIOS_SIZE = 512*1024; // 512KB bios size

int main() {

    if (!file_exists(BIOS_FNAME))
    {
        DEBUG("BIOS not found. Expected path: " << BIOS_FNAME);
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);

    Bios bios = Bios(BIOS_FNAME, BIOS_SIZE);
    Ram ram = Ram();
    Dma dma = Dma();
    Gpu gpu = Gpu();
    Spunit spu = Spunit(); // SPU is a reserved keyword??

    Interconnect interconnect = Interconnect(&bios, &ram, &dma, &gpu, &spu);

    Cpu cpu = Cpu(&interconnect);

    // Main Loop
    SDL_Event e; 
    while (true) // <3
    {
        // only check for events every 50k cpu instructions
        uint32_t instructions_run = 0;
        while (instructions_run <= 50000)
        {
            cpu.runNextInstruction();
            instructions_run++;
        }

        // check for events
        SDL_PollEvent(&e);
        switch (e.type)
        {
            case SDL_WINDOWEVENT: // Window closed
                switch (e.window.event) 
                {
                    case SDL_WINDOWEVENT_CLOSE:  
                        DEBUG("Window closed")
                        return 0;
                        break;
                    default: break;
                }
                break; 
            case SDL_QUIT: // sigint etc.
                return 0;
                break;              
            default: break;
        }
    }

    return 0;
}

