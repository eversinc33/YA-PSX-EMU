cmake_minimum_required(VERSION 3.17)
project(PSXEMU)

set(CMAKE_CXX_STANDARD 14)

add_executable(PSXEMU main.cpp core/cpu/Cpu.cpp core/cpu/Cpu.h core/Bios.cpp core/Bios.h core/Interconnect.cpp core/Interconnect.h core/memory/Range.cpp core/memory/Range.h core/cpu/Instruction.cpp core/cpu/Instruction.h core/memory/Ram.cpp core/memory/Ram.h)