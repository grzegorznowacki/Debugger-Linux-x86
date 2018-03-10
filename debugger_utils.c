//
// Created by gnowacki on 10.03.18.
//

#include "debugger_utils.h"

void stepi(pid_t child_pid, int wait_status, unsigned int* counter)
{
    if(WIFSTOPPED(wait_status))
    {
        (*counter)++;
        struct user_regs_struct registers;
        ptrace(PTRACE_GETREGS, child_pid, 0, &registers);
        unsigned int eip = registers.eip;

        if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
            perror("ptrace");
            return;
        }

        wait(&wait_status);

        ptrace(PTRACE_GETREGS, child_pid, 0, &registers);
        unsigned int next_eip = registers.eip;

        printf("Executed instruction [%d]: ", *counter);
        print_instruction_opcode(child_pid, eip, next_eip);
    }
}

void info_registers(pid_t child_pid)
{
    struct user_regs_struct registers;
    ptrace(PTRACE_GETREGS, child_pid, 0, &registers);
    print_registers(&registers);
}

void print_registers(const struct user_regs_struct* regs) {
    printf("EIP = 0x%08x\n", regs->eip);
    printf("EBX = 0x%08x    ECX = 0x%08x    EDX = 0x%08x    EAX = 0x%08x\n", regs->ebx, regs->ecx, regs->edx, regs->eax);
    printf("ESI = 0x%08x    EDI = 0x%08x    EBP = 0x%08x    ESP = 0x%08x\n", regs->esi, regs->edi, regs->ebp, regs->esp);
}

void print_instruction_opcode(pid_t pid, unsigned int from_addr, unsigned int to_addr)
{
    printf("EIP: 0x%08X   OPCODE: ", from_addr);
    for(unsigned int addr = from_addr; addr < to_addr; ++addr)
    {
        unsigned int word = ptrace(PTRACE_PEEKTEXT, pid, addr, 0);
        printf("%02x", word & 0xFF);
    }
    printf("\n");
}

