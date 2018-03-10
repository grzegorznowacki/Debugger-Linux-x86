cmake_minimum_required(VERSION 3.9)
project(Debugger_Linux_x86 C)

set(CMAKE_C_STANDARD 99)

add_executable(Debugger_Linux_x86 main.c)

target_link_libraries(Debugger_Linux_x86 elf dwarf)