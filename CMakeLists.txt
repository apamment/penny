cmake_minimum_required(VERSION 3.0)

project(penny)

add_subdirectory(magidoor)

add_custom_command(
    OUTPUT ROM.o
    COMMAND ${CMAKE_OBJCOPY} ARGS -I ihex -O binary ${CMAKE_CURRENT_SOURCE_DIR}/osi_bas/ROM.HEX ROM.bin
    COMMAND ${CMAKE_OBJCOPY} ARGS -I binary -O elf64-x86-64 -B i386:x86-64 --rename-section .data=.rodata,alloc,load,readonly,data,contents ROM.bin ROM.o
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/osi_bas/ROM.HEX
    VERBATIM
)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_executable(penny main.c fake6502.c fake6850.c ROM.o)
    TARGET_LINK_LIBRARIES(penny mdoor Ws2_32)
else()
    add_executable(penny main.c fake6502.c fake6850.c ROM.o)
    TARGET_LINK_LIBRARIES(penny mdoor)
endif()

