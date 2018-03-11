//
// Created by gnowacki on 10.03.18.
//

#include "debugger_utils.h"

int stepi(pid_t child_pid, int* wait_status, unsigned int* counter, breakpoint_struct** breakpoint_array, int* insert_elem)
{
    if(WIFSTOPPED(*wait_status))
    {
        breakpoint_struct* breakpoint = NULL;

        breakpoint = check_if_breakpoint(child_pid, breakpoint_array, insert_elem);

        (*counter)++;
        struct user_regs_struct registers;
        ptrace(PTRACE_GETREGS, child_pid, 0, &registers);
        unsigned int eip = registers.eip;           //TODO long int should be

        if((breakpoint != NULL) && ((ptrace(PTRACE_PEEKTEXT, child_pid, breakpoint->address, 0) & 0xFF) == 0xCC))
        {
            disable_breakpoint(child_pid, breakpoint);
        }

        if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
            perror("ptrace");
            return -1;
        }

        wait(wait_status);  //NOTE that wait_status is passed through pointer

        ptrace(PTRACE_GETREGS, child_pid, 0, &registers);
        unsigned int next_eip = registers.eip;

        printf("Executed instruction [%d]: ", *counter);
        print_instruction_opcode(child_pid, eip, next_eip);

        if(breakpoint != NULL)
        {
            enable_breakpoint(child_pid, breakpoint);
        }

        return 0;
    }
    else if(WIFEXITED(*wait_status))
    {
       return 1;
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
int run(pid_t child_pid, int* wait_status, breakpoint_struct** breakpoint_array, int* insert_elem)
{
    ptrace(PTRACE_CONT, child_pid, 0, 0);
    wait(wait_status);

    if (WIFEXITED(*wait_status))
        return 0;

    clean_breakpoint_and_stepback(child_pid, wait_status, breakpoint_array, insert_elem);

    if (WIFSTOPPED(*wait_status))
        return 1;
}

breakpoint_struct* check_if_breakpoint(pid_t child_pid, breakpoint_struct** breakpoint_array, int* insert_elem)
{
    breakpoint_struct* breakpoint = NULL;
    struct user_regs_struct registers;

    ptrace(PTRACE_GETREGS, child_pid, 0, &registers);

    int i;
    for(i = 0; i < MAX_BREAKPOINTS; ++i)
    {
        if((breakpoint_array[i] != NULL) && ((long)breakpoint_array[i]->address == registers.eip))  //IMPORTANT - here is no address + 1 as in clean_breakpoint_and_stepback()
        {
            breakpoint = breakpoint_array[i];
            return breakpoint;
        }
    }
}

int continue_debugging(pid_t child_pid, int* wait_status, breakpoint_struct** breakpoint_array, int* insert_elem)
{
    breakpoint_struct* breakpoint = NULL;

    breakpoint = check_if_breakpoint(child_pid, breakpoint_array, insert_elem);

    if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0)
    {
        perror("ptrace");
        return -1;
    }

    wait(wait_status);

    if (WIFEXITED(*wait_status))
        return 0;

    if(breakpoint != NULL)  //if was not found in the array - means that someone has deleted breakpoint in the meanwhile
        enable_breakpoint(child_pid, breakpoint);

    ptrace(PTRACE_CONT, child_pid, 0, 0);
    wait(wait_status);

    if (WIFEXITED(*wait_status))
        return 0;

    clean_breakpoint_and_stepback(child_pid, wait_status, breakpoint_array, insert_elem);

    if (WIFSTOPPED(*wait_status))
        return 1;
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

void free_breakpoint_array(breakpoint_struct** breakpoint_array)
{
    int i;
    for(i = 0; i < MAX_BREAKPOINTS; ++i)
    {
        free(breakpoint_array[i]);
    }
}

void break_at_address(pid_t child_pid, int* wait_status, const char* command_name, breakpoint_struct** breakpoint_array, int* insert_elem)
{
    char address_array[8];
    strncpy(address_array, command_name + 8, 8);    //TODO ??? chyba ok
    unsigned int address = (unsigned int)strtol(address_array, NULL, 16);   //TODO ??? chyba ok
    //TODO long zamiast unsigned int??? na 32 bitach to chyba to samo

    breakpoint_struct* breakpoint = create_breakpoint(child_pid, (void*)address);
    breakpoint_array[*insert_elem] = breakpoint;
    (*insert_elem)++;
    //TODO wtablice wskznikow zrobic
    //TODO i ogolnie dokonczyc bo na razei zrobilem tylko create
    //TODO tu bedzie trzeb zrobic to co jest w resume from breakpoint!!!
    //TODO + patrz to co na kartce napisalem
    //TODO TO NIE TU A W RUN I CONTINUE PO WAIT TRZEBA JEDNAK IMPLEMENTOWAC!!!
    //i moze tez w stepi itd !!! plus ta tablica do zrobienia i sprwadzenia!!
}

//TODO change to void probably
int clean_breakpoint_and_stepback(pid_t pid, int* wait_status, breakpoint_struct** breakpoint_array, int* insert_elem)
{
    breakpoint_struct* breakpoint;
    struct user_regs_struct registers;

    ptrace(PTRACE_GETREGS, pid, 0, &registers);

    int i;
    for(i = 0; i < MAX_BREAKPOINTS; ++i)
    {
        if((breakpoint_array[i] != NULL) && ((long)breakpoint_array[i]->address + 1 == registers.eip))
        {
            breakpoint = breakpoint_array[i];
            break;
        }
    }

    registers.eip = (long) breakpoint->address;
    ptrace(PTRACE_SETREGS, pid, 0, &registers);
    disable_breakpoint(pid, breakpoint);
}

void info_break(pid_t child_pid, breakpoint_struct** breakpoint_array, int* insert_elem)
{
    int is_any = 0;
    int i;

    printf("Enabled breakpoints: \n");

    for(i = 0; i < MAX_BREAKPOINTS; ++i)
    {
        if(breakpoint_array[i] != NULL)
        {
            printf("[%d] 0x%08X\n", i, breakpoint_array[i]->address);
            is_any = 1;
        }
    }

    if(is_any == 0)
        printf("%s\n", "No breakpoints are set");
}