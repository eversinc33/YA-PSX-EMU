#include <stdint.h>
#include "Bios.h"
#include "Interconnect.h"
#include "Cpu.h"

const char* FNAME = "/home/sven/CLionProjects/PSXEMU/SCPH1001.BIN";
const uint32_t BUFFERSIZE = 512*1024; // 512KB bios size

int main() {
    Bios bios = Bios(FNAME, BUFFERSIZE);

    Interconnect interconnect = Interconnect(&bios);

    Cpu cpu = Cpu(&interconnect);

    while (1) {
        cpu.runNextInstruction();
    }
}

