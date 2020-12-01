#include <cstdint>
#include "bios/Bios.h"
#include "cpu/Cpu.h"
#include "memory/Ram.h"

const char* FNAME = "/home/sven/CLionProjects/PSXEMU/SCPH1001.BIN";
const uint32_t BUFFERSIZE = 512*1024; // 512KB bios size

int main() {
    Bios bios = Bios(FNAME, BUFFERSIZE);
    Ram ram = Ram();

    Interconnect interconnect = Interconnect(&bios, &ram);

    Cpu cpu = Cpu(&interconnect);

    while (1) {
        cpu.runNextInstruction();
    }
}

