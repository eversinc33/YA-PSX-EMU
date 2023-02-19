cmake_minimum_required(VERSION 3.17)
project(PSXEMU)

set(CMAKE_CXX_STANDARD 14)

FIND_PACKAGE(SDL2 REQUIRED)
Message("")
IF (${SDL2_FOUND})
    Message(STATUS "SDL2_INCLUDE_DIR:" ${SDL2_INCLUDE_DIR})
    Message(STATUS "SDL2_LIBRARY: " ${SDL2_LIBRARY})
ELSE()
    Message(FATAL_ERROR "SDL2 NOT FOUND")
ENDIF()

include_directories(PSXEMU ${SDL2_INCLUDE_DIRS})

add_executable(PSXEMU
    main.cpp
    cpu/Cpu.cpp
    cpu/Cpu.h
    bios/Bios.cpp
    bios/Bios.h
    bus/Interconnect.cpp
    bus/Interconnect.h
    memory/Range.cpp
    memory/Range.h
    cpu/Instruction.cpp
    cpu/Instruction.h
    memory/Ram.cpp
    memory/Ram.h
    cpu/Opcodes.cpp
    memory/MemoryMap.h
    memory/Dma.cpp
    memory/Dma.h
    memory/Channel.cpp
    memory/Channel.h
    util/logging.h
    gpu/Gpu.cpp
    gpu/Gpu.h
    gpu/Renderer.cpp
    gpu/Renderer.h
    gpu/Constants.h
)

target_link_libraries(PSXEMU SDL2::SDL2) # ${SDL2_LIBRARY})