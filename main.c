#include <libdwarf.h>
#include <fcntl.h>
#include "debugger_utils.h"
#include "dwarf_utils.h"


void run_debuggee_proc(const char* prog_name)
{
    printf("%s\n", "Child proc");
    if(ptrace(PT_TRACE_ME, 0, 0, 0) < 0)
    {
        perror("Ptrace error");
        return;
    }

    execl(prog_name, prog_name, 0);
}

void nullify_array(breakpoint_struct** breakpoint_array)
{
    int i;
    for(i = 0; i < MAX_BREAKPOINTS; ++i)
    {
        breakpoint_array[i] = NULL;
    }
}

void run_debugger_proc(pid_t child_pid, const char* child_prog_name)
{
    char is_first_run = 1;  //treated as boolean - missing in C
    int wait_status;
    unsigned int counter = 0;
    char command_name[COMMAND_LEN];

    breakpoint_struct* breakpoint_array[MAX_BREAKPOINTS];
    int  insert_elem = 0;
    nullify_array(breakpoint_array);

    //DWARF SECTION
    Dwarf_Debug dbg = 0;
    Dwarf_Error err;
    int file_desc = -1;

    if((file_desc = open(child_prog_name, O_RDONLY)) < 0)
    {
        perror("open");
        return;
    }
    //END OF DWARF SECTION

    printf("%s\n", "Parent proc");

    wait(&wait_status);

    printf("(deb)");

    while(fgets(command_name, COMMAND_LEN, stdin))
    {
        if(strcmp(command_name, "quit\n") == 0) //IMPORTANT - \n - because fgets puts also LF into command_name
        {
            free_breakpoint_array(breakpoint_array);

            //DWARF SECTION
            close(file_desc);
            //END OF DWARF SECTION

            printf("%s", "Exiting debugger\n");
            return;
        }
        else if(strcmp(command_name, "run\n") == 0)
        {
            if(is_first_run == 1)
            {
                int ret_status = run(child_pid, &wait_status, breakpoint_array, &insert_elem);
                is_first_run = 0;
                if (ret_status == 0)
                {
                    printf("Debuggee program has ended\n\n");
                }
            }
            else
            {
                run_new(child_prog_name);
            }
        }
        else if(strcmp(command_name, "continue\n") == 0)
        {
            if(is_first_run == 1)
            {
                printf("%s", "<continue> not possible here - use <run> instead");
                is_first_run = 0;
            }
            else
            {
                int ret_status = continue_debugging(child_pid, &wait_status, breakpoint_array, &insert_elem);

                if (ret_status == 0)
                {
                    printf("Debuggee program has ended\n\n");
                }
            }
        }
        else if(strcmp(command_name, "stepi\n") == 0)
        {
            int ret_status = stepi(child_pid, &wait_status, &counter, breakpoint_array, &insert_elem);
            if(ret_status == 1)
                printf("Debugee program has ended\n");
        }
        else if(strncmp(command_name, "break ", 6) == 0)    //IMPORTANT - space after break
        {
            if(strncmp(command_name, "break 0x", 8) == 0)
            {
                break_at_address(child_pid, command_name, breakpoint_array, &insert_elem);
            }

            if(strncmp(command_name, "break line ", 11) == 0)
            {
                DWARF_INIT()
                break_at_line(dbg, child_pid, command_name, breakpoint_array, &insert_elem);
                DWARF_FINISH()
            }

            if(strncmp(command_name, "break function ", 15) == 0)
            {
                DWARF_INIT()
                break_at_function(dbg, child_pid, command_name, breakpoint_array, &insert_elem);
                DWARF_FINISH()
            }
        }
        else if(strcmp(command_name, "info registers\n") == 0)
        {
            info_registers(child_pid);
        }
        else if(strncmp(command_name, "del ", 4) == 0)
        {
            int break_num = del_breakpoint(child_pid, &wait_status, command_name, breakpoint_array, &insert_elem);
            if(break_num != -1)
            {
                printf("%s%d\n", "Deleted breakpoint: ", break_num);
            }
            else
            {
                printf("%s\n", "No such breakpoint");
            }
        }
        else if(strcmp(command_name, "info break\n") == 0)
        {
            info_break(child_pid, breakpoint_array, &insert_elem);
        }
        else if(strcmp(command_name, "info functions\n") == 0)
        {
            DWARF_INIT()
            list_functions_with_address(dbg);
            DWARF_FINISH()
        }
        else if(strcmp(command_name, "info lines\n") == 0)
        {
            DWARF_INIT()
            line_address_mapping(dbg);
            DWARF_FINISH()
        }

        printf("(deb)");
    }
}

int main(int argc, char** argv)
{
    pid_t child_pid;

    if(argc < 2)
    {
        printf("%s", "Expected program name to be debugged passed as argument");
        return 1;
    }

    child_pid = fork();

    if(child_pid == 0)
        run_debuggee_proc(argv[1]);
    else if(child_pid > 0)
        run_debugger_proc(child_pid, argv[1]);
    else
    {
        perror("Fork error");
        return 2;
    }

    return 0;
}