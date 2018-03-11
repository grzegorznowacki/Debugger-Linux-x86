#include "debugger_utils.h"

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

    printf("%s\n", "Parent proc");

    wait(&wait_status);

    printf("(deb)");

    while(fgets(command_name, COMMAND_LEN, stdin))
    {
        //printf("%s", command_name);
        //printf("%d", strcmp(command_name, "quit\n"));
        //TODO Jak wyjdziemy z debuggera to osierocimy dziecko.
        //TODO Ale w sumie to malo wazne bo wtedy dziecko zostaje
        //TODO zaadoptowane przez proces init bodajze i on go usunie
        //TODO czy jakos tak. Sasza cos o tym chyba wspominal.
        //TODO Wiec sie tym nie martwimy, co nie???
        if(strcmp(command_name, "quit\n") == 0) //IMPORTANT - \n - because fgets puts also LF into command_name
        {
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
        else if(strcmp(command_name, "step\n") == 0)
        {

        }
        else if(strcmp(command_name, "stepi\n") == 0)
        {
            int ret_status = stepi(child_pid, &wait_status, &counter);
            if(ret_status == 1)
                printf("Debugee program has ended\n");
        }
        else if(strcmp(command_name, "next\n") == 0)
        {

        }
        else if(strncmp(command_name, "break ", 6) == 0)    //IMPORTANT - space after break
        {
            if(strncmp(command_name, "break 0x", 8) == 0)
            {
                break_at_address(child_pid, &wait_status, command_name, breakpoint_array, &insert_elem);
            }
            //TODO przypadek - linia w kodzie - to chyba moze skorzystac z funkcji break_at_address()
            //TODO przypadek - nazwa funkcji
        }
        else if(strcmp(command_name, "info registers\n") == 0)
        {
            info_registers(child_pid);
        }
        else if(strcmp(command_name, "clear \n") == 0)  //TODO clear line_num
        {

        }
        else if(strcmp(command_name, "del \n") == 0)    //TODO del breakpt_num
        {

        }
        else if(strcmp(command_name, "info break\n") == 0)    //TODO show breakpoints
        {
            info_break(child_pid, breakpoint_array, &insert_elem);
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