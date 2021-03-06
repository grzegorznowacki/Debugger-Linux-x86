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

/* IMPORTANT - These macros should always be used before and after a function which parses DWARF sections */
#define DWARF_INIT() if(dwarf_init(file_desc, DW_DLC_READ, 0, 0, &dbg, &err) != DW_DLV_OK) {printf("%s", "DWARF init failed\n"); exit(EXIT_FAILURE);}
#define DWARF_FINISH() if(dwarf_finish(dbg, &err) != DW_DLV_OK) {printf("%s", "DWARF finish failed\n"); exit(EXIT_FAILURE);}

void show_function_address_and_line(Dwarf_Debug dgb, Dwarf_Die the_die);

void list_functions_with_address(Dwarf_Debug dbg);

Dwarf_Addr find_function_address(Dwarf_Debug dgb, Dwarf_Die the_die, char* function_name);

void break_at_function(Dwarf_Debug dbg, pid_t child_pid, const char* command_name, breakpoint_struct** breakpoint_array, int* insert_elem);

void break_at_line(Dwarf_Debug dbg, pid_t child_pid, const char* command_name, breakpoint_struct** breakpoint_array, int* insert_elem);

void line_address_mapping(Dwarf_Debug dbg);

#endif //DEBUGGER_LINUX_X86_DWARF_UTILS_H
