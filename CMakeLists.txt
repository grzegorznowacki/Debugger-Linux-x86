cmake_minimum_required(VERSION 3.9)
project(Debugger_Linux_x86 C)

set(CMAKE_C_STANDARD 99)

add_executable(Debugger_Linux_x86 main.c debugger_utils.c debugger_utils.h dwarf_utils.c dwarf_utils.h)

target_link_libraries(Debugger_Linux_x86 elf dwarf)
set_target_properties(Debugger_Linux_x86 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")