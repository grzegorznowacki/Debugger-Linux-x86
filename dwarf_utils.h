//
// Created by gnowacki on 21.03.18.
//

#ifndef DEBUGGER_LINUX_X86_DWARF_UTILS_H
#define DEBUGGER_LINUX_X86_DWARF_UTILS_H

#include "debugger_utils.h"

void break_at_function(pid_t child_pid, int* wait_status, const char* command_name, breakpoint_struct** breakpoint_array, int* insert_elem);

#endif //DEBUGGER_LINUX_X86_DWARF_UTILS_H
