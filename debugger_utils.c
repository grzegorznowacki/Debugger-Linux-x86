//
// Created by gnowacki on 10.03.18.
//

#include "debugger_utils.h"

void stepi(pid_t child_pid, int* wait_status, unsigned int* counter)
{
    if(WIFSTOPPED(*wait_status))
    {
        (*counter)++;
        struct user_regs_struct registers;
        ptrace(PTRACE_GETREGS, child_pid, 0, &registers);
        unsigned int eip = registers.eip;

        if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
            perror("ptrace");
            return;
        }

        wait(wait_status);  //NOTE that wait_status is passed through pointer

        ptrace(PTRACE_GETREGS, child_pid, 0, &registers);
        unsigned int next_eip = registers.eip;

        printf("Executed instruction [%d]: ", *counter);
        print_instruction_opcode(child_pid, eip, next_eip);
    }
    else if(WIFEXITED(*wait_status))
    {
        printf("Debugee program has ended\n");
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

extern void run_debuggee_proc(const char* prog_name);
extern void run_debugger_proc(pid_t child_pid, const char* child_prog_name);

void run_new(const char* child_prog_name)
{
    pid_t child_pid = fork();
    if(child_pid == 0)
        run_debuggee_proc(child_prog_name);
    else if(child_pid > 0)
        run_debugger_proc(child_pid, child_prog_name);
    else
    {
        perror("Fork error");
        return;
    }
}

//TODO run - nie jestem pewien czy to wystarczy
//TODO moze cos jeszcze z makrami WIF...
void run(pid_t child_pid, int* wait_status)
{
    ptrace(PTRACE_CONT, child_pid, 0, 0);
    wait(wait_status);
}

void enable_breakpoint(pid_t pid, breakpoint_struct* breakpoint)
{
    assert(breakpoint);
    breakpoint->original_data = ptrace(PTRACE_PEEKTEXT, pid, breakpoint->address, 0);
    ptrace(PTRACE_POKETEXT, pid, breakpoint->address, (breakpoint->original_data & 0xFFFFFF00) | 0xCC);
}

void disable_breakpoint(pid_t pid, breakpoint_struct* breakpoint)
{
    assert(breakpoint);
    unsigned int data = ptrace(PTRACE_PEEKTEXT, pid, breakpoint->address, 0);
    assert((data & 0xFF) == 0xCC);
    ptrace(PTRACE_POKETEXT, pid, breakpoint->address, (data & 0xFFFFFF00) | (breakpoint->original_data & 0xFF));
}

breakpoint_struct* create_breakpoint(pid_t pid, void* addr)
{
    breakpoint_struct* breakpoint = malloc(sizeof(breakpoint_struct));
    breakpoint->address = addr;
    enable_breakpoint(pid, breakpoint);
    return breakpoint;
}

void free_breakpoint(breakpoint_struct* breakpoint)
{
    free(breakpoint);
}

void break_at_address(pid_t child_pid, int* wait_status, const char* command_name, breakpoint_struct** breakpoint_array, int* insert_elem)
{
    char address_array[8];
    strncpy(address_array, command_name + 8, 8);    //TODO ???
    unsigned int address = (unsigned int)strtol(address_array, NULL, 16);   //TODO ???

    breakpoint_struct* breakpoint = create_breakpoint(child_pid, (void*)address);

    (*insert_elem)++;
    //TODO wtablice wskznikow zrobic
    //TODO i ogolnie dokonczyc bo na razei zrobilem tylko create
    //TODO tu bedzie trzeb zrobic to co jest w resume from breakpoint!!!
    //TODO + patrz to co na kartce napisalem
}