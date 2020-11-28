cmake_minimum_required(VERSION 3.17)
project(PSXEMU)

set(CMAKE_CXX_STANDARD 14)

add_executable(PSXEMU main.cpp Cpu.cpp Cpu.h Bios.cpp Bios.h Interconnect.cpp Interconnect.h Range.cpp Range.h)