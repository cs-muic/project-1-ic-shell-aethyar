/* ICCS227: Project 1: icsh
 * Name: Adrian Jay Ang
 * StudentID: 6380989
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "fcntl.h"

#define MAX_CMD_BUFFER 255

int script_mode = 0;
int prev_exit = 0;
pid_t fg_process = 0;

void signal_handler(int sig)
{
    if (fg_process != 0)
    {
        kill(fg_process, sig);
    }
}

void external_cmd(char *cmd)
{
    int status;
    int pid;

    if ((pid = fork()) < 0)
    {
        perror("Fork failed");
        exit(1);
    }
    if (!pid)
    {
        char *args[MAX_CMD_BUFFER];
        char *separator = " \t\n";
        char *token = strtok(cmd, separator);
        int i = 0;
        while (token != NULL) 
        {
            args[i++] = token;
            token = strtok(NULL, separator);
        }
        args[i] = NULL;

        int i_redir = -1;
        int o_redir = -1;

        for (int j = 0; args[j] != NULL; j++) 
        {
            if (strcmp(args[j], "<") == 0) 
            {
                i_redir = open(args[j + 1], O_RDONLY);
                if (i_redir == -1) 
                {
                    perror("Error with input file");
                    exit(1);
                }
                args[j] = NULL;
            } 
            else if (strcmp(args[j], ">") == 0) 
            {
                o_redir = open(args[j + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (o_redir == -1) 
                {
                    perror("Error with output file");
                    exit(1);
                }
                args[j] = NULL;
            }
        }

        if (i_redir != -1) {
            if (dup2(i_redir, STDIN_FILENO) == -1) 
            {
                perror("Failed to redirect input");
                exit(1);
            }
            close(i_redir);
        }
        if (o_redir != -1) {
            if (dup2(o_redir, STDOUT_FILENO) == -1) 
            {
                perror("Failed to redirect output");
                exit(1);
            }
            close(o_redir);
        }

        execvp(args[0], args);
        exit(1);
    }
    if (pid)
    {
        fg_process = pid;
        waitpid(pid, &status, 0);
        fg_process = 0;

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        {
            printf("bad command\n");
        }
    }
}

void process_cmd(char *cmd)
{
    static char prev_cmd[MAX_CMD_BUFFER] = {0};
    cmd[strcspn(cmd, "\r\n")] = 0;
    if (strlen(cmd) != 0)
    {
        if (strcmp(cmd, "echo $?") == 0)
        {
            printf("%d\n", prev_exit);
        }
        else if (strcmp(cmd, "!!") == 0)
        {
            if (strlen(prev_cmd) != 0)
            {
                if (script_mode == 0)
                {
                    printf("%s\n", prev_cmd);
                }
                process_cmd(prev_cmd);
            }
            return;
        }
        else if (strncmp(cmd, "exit ", 5) == 0)
        {
            int exit_code = atoi(cmd + 5);
            if (exit_code > 255)
            {
                exit_code = 255;
            }

            if (script_mode == 1)
            {
                prev_exit = exit_code;
            }
            else
            {
                printf("Goodbye\n");
                exit(exit_code);
            }
        }
        else
        {
            external_cmd(cmd);
        }
        strncpy(prev_cmd, cmd, MAX_CMD_BUFFER);
    }
}

int main(int argc, char *argv[])
{
    printf("Starting IC shell\n");
    char buffer[MAX_CMD_BUFFER];

    signal(20, signal_handler);
    signal(2, signal_handler);

    if (argc > 1)
    {
        script_mode = 1;
        FILE *script_file = fopen(argv[1], "r");
        if (script_file == NULL)
        {
            printf("Error reading argument\n");
        }
        else
        {
            fgets(buffer, MAX_CMD_BUFFER, script_file);
            while (fgets(buffer, MAX_CMD_BUFFER, script_file))
            {
                process_cmd(buffer);
            }
            fclose(script_file);
        }
        script_mode = 0;
    }
    while (1)
    {
        printf("icsh $ ");
        fgets(buffer, MAX_CMD_BUFFER, stdin);
        process_cmd(buffer);
    }
}
