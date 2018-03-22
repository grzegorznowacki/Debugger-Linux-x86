//
// Created by gnowacki on 21.03.18.
//

#ifndef DEBUGGER_LINUX_X86_DWARF_UTILS_H
#define DEBUGGER_LINUX_X86_DWARF_UTILS_H

#include <libdwarf.h>
#include <dwarf.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "debugger_utils.h"

void show_function_address_and_line(Dwarf_Debug dgb, Dwarf_Die the_die);

void list_functions_with_address(Dwarf_Debug dbg, pid_t child_pid, int* wait_status, const char* command_name, breakpoint_struct** breakpoint_array, int* insert_elem);

#endif //DEBUGGER_LINUX_X86_DWARF_UTILS_H
