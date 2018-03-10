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

#define COMMAND_LEN 30

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

void run_debugger_proc(pid_t child_pid)
{
    int wait_status;
    char command_name[COMMAND_LEN];  //TODO - overflow

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

        }
        else if(strcmp(command_name, "continue\n") == 0)
        {

        }
        else if(strcmp(command_name, "step\n") == 0)
        {

        }
        else if(strcmp(command_name, "next\n") == 0)
        {

        }
        else if(strncmp(command_name, "break ", 6) == 0)    //IMPORTANT - space after break
        {

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
        run_debugger_proc(child_pid);
    else
    {
        perror("Fork error");
        return 2;
    }

    return 0;
}