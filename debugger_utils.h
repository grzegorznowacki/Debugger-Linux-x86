//
// Created by gnowacki on 10.03.18.
//

#ifndef DEBUGGER_LINUX_X86_DEBUGGER_UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

typedef struct breakpoint_struct_t breakpoint_struct;

struct breakpoint_struct_t
{
    void* address;
    unsigned int original_data;
};

void enable_breakpoint(pid_t pid, breakpoint_struct* breakpoint);
void disable_breakpoint(pid_t pid, breakpoint_struct* breakpoint);
breakpoint_struct* create_breakpoint(pid_t pid, void* addr);
void free_breakpoint(breakpoint_struct* breakpoint);

void stepi(pid_t child_pid, int* wait_status, unsigned int* counter);

void print_registers(const struct user_regs_struct* regs);

void print_instruction_opcode(pid_t pid, unsigned int from_addr, unsigned int to_addr);

void info_registers(pid_t child_pid);

void run(pid_t child_pid, int* wait_status);

void run_new(const char* child_prog_name);

void break_address(pid_t child_pid, int* wait_status);

#define DEBUGGER_LINUX_X86_DEBUGGER_UTILS_H

#endif //DEBUGGER_LINUX_X86_DEBUGGER_UTILS_H
