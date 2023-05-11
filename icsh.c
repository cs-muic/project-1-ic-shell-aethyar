/* ICCS227: Project 1: icsh
 * Name: Adrian Jay Ang
 * StudentID: 6380989
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define MAX_CMD_BUFFER 255

void process_cmd(char* cmd)
{
    static char prev_cmd[MAX_CMD_BUFFER] = {0};

    if (strncmp(cmd, "echo ", 5) == 0)
    {
        printf("%s\n", cmd + 5);
    }
    else if (strcmp(cmd, "!!") == 0)
    {
        if (strlen(prev_cmd) != 0) 
        {
            printf("%s\n", prev_cmd);
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
        printf("Goodbye\n");
        exit(exit_code);
    }
    else
    {
        printf("bad command\n");
        return;
    }
    
    strncpy(prev_cmd, cmd, MAX_CMD_BUFFER);
}

int main(int argc, char *argv[])
{
    printf("Starting IC shell\n");
    char buffer[MAX_CMD_BUFFER];

    if (argc > 1)
    {
        FILE* script_file = fopen(argv[1], "r");
        if (script_file == NULL) 
        {
            printf("Error reading argument\n");
        }
        else 
        {
            fgets(buffer, MAX_CMD_BUFFER, script_file);
            while (fgets(buffer, MAX_CMD_BUFFER, script_file))
            {
                buffer[strcspn(buffer, "\r\n")] = 0;
                if (strlen(buffer) != 0)
                {
                    process_cmd(buffer);
                }
            }
            fclose(script_file);
        }

    }
    while (1) 
    {
        printf("icsh $ ");
        fgets(buffer, 255, stdin);

        buffer[strcspn(buffer, "\r\n")] = 0;
        if (strlen(buffer) != 0)
        {
            process_cmd(buffer);
        }
    }
}
