#include <cstdint>
#include "bios/Bios.h"
#include "cpu/Cpu.h"
#include "gpu/Gpu.h"
#include "memory/Ram.h"
#include "util/logging.h"

const char* FNAME = "./SCPH1001.BIN";
const uint32_t BUFFERSIZE = 512*1024; // 512KB bios size

int main() {
    Bios bios = Bios(FNAME, BUFFERSIZE);
    Ram ram = Ram();
    Dma dma = Dma();

    Interconnect interconnect = Interconnect(&bios, &ram, &dma);

    Cpu cpu = Cpu(&interconnect);
    Gpu gpu = Gpu();

    while (1) {
        cpu.runNextInstruction();
    }
}

