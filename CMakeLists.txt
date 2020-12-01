cmake_minimum_required(VERSION 3.17)
project(PSXEMU)

set(CMAKE_CXX_STANDARD 14)

add_executable(PSXEMU main.cpp cpu/Cpu.cpp cpu/Cpu.h bios/Bios.cpp bios/Bios.h bus/Interconnect.cpp bus/Interconnect.h memory/Range.cpp memory/Range.h cpu/Instruction.cpp cpu/Instruction.h memory/Ram.cpp memory/Ram.h cpu/Opcodes.cpp memory/MemoryMap.h)