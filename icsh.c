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
#define MAX_JOBS 100

int script_mode = 0;
int prev_exit = 0;
pid_t fg_process = 0;

typedef struct 
{
    pid_t pid;
    char cmd[MAX_CMD_BUFFER];
    int status;
} Job;

Job jobs[MAX_JOBS];
int job_count = 0;

void external_cmd(char *cmd)
{
    int status;
    int pid;

    if ((pid = fork()) < 0)
    {
        perror("Fork failed");
        exit(1);
    }
    
    int bg = 0;
    char orig_cmd[MAX_CMD_BUFFER];
    strcpy(orig_cmd, cmd);
    if (cmd[strlen(cmd) - 1] == '&')
    {
        bg = 1;
        cmd[strlen(cmd) - 1] = '\0';
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

        if (i_redir != -1) 
        {
            if (dup2(i_redir, STDIN_FILENO) == -1) 
            {
                perror("Failed to redirect input");
                exit(1);
            }
            close(i_redir);
        }
        if (o_redir != -1)
        {
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
        if (bg == 1)
        {
            printf("[%d] %d\n", job_count+1, pid);
            jobs[job_count].pid = pid;
            strncpy(jobs[job_count].cmd, orig_cmd, MAX_CMD_BUFFER);
            jobs[job_count].status = 1;
            job_count++;
        }
        else
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
        else if (strncmp(cmd, "jobs", 4) == 0)
        {
            for (int i = 0; i < job_count; i++)
            {
                int status;
                pid_t result = waitpid(jobs[i].pid, &status, WNOHANG | WUNTRACED);
                if (result == 0)
                {
                    printf("[%d]-  Running           %s\n", i+1, jobs[i].cmd);
                }
                else if (WIFSTOPPED(status) || jobs[i].status == 0)
                {
                    printf("[%d]+  Stopped           %s\n", i+1, jobs[i].cmd);
                }
            }
        }
        else if (strncmp(cmd, "fg %%", 4) == 0)
        {
            int job_id = atoi(cmd+4);
            if (job_id > 0 && job_id <= job_count)
            {
                printf("%s\n", jobs[job_id-1].cmd);
                pid_t job_pid = jobs[job_id-1].pid;
                fg_process = job_pid;
                waitpid(job_pid, &prev_exit, 0);
                fg_process = 0;
            }
            else
            {
                printf("Invalid job ID\n");
            }
        }
        else if (strncmp(cmd, "bg %%", 4) == 0)
        {
            int job_id = atoi(cmd+4);
            if (job_id > 0 && job_id <= job_count)
            {
                pid_t job_pid = jobs[job_id-1].pid;
                jobs[job_id-1].status = 0;
                kill(job_pid, SIGCONT);
            }
            else
            {
                printf("Invalid job ID\n");
            }
        }
        else
        {
            external_cmd(cmd);
        }
        strncpy(prev_cmd, cmd, MAX_CMD_BUFFER);
    }
}

void check_background_jobs()
{
    int status;
    pid_t terminated_pid;

    while ((terminated_pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = 0; i < job_count; i++)
        {
            if (jobs[i].pid == terminated_pid)
            {
                printf("\n[%d]+  Done              %s\n", i + 1, jobs[i].cmd);
                printf("icsh $ ");
                fflush(stdout);
                for (int j = i; j < job_count - 1; j++)
                {
                    jobs[j] = jobs[j + 1];
                }
                job_count--;
                break;
            }
        }
    }
}

void signal_handler(int sig)
{
    if (fg_process != 0)
    {
        kill(fg_process, sig);
    }
    printf("\n");
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    printf("Starting IC shell\n");
    char buffer[MAX_CMD_BUFFER];

    struct sigaction action = { 0 };
    action.sa_handler = signal_handler;
    sigaction(SIGTSTP, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    signal(SIGCHLD, check_background_jobs);

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
        fflush(stdout);
        fgets(buffer, MAX_CMD_BUFFER, stdin);
        process_cmd(buffer);
    }
}
