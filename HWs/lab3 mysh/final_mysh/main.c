//
//  Description: (sh Simulator) Simulate Unix sh for command processing
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "header.h"

extern char*   PATH;                 /* Pointer to PATH ENV variable */
extern char*   HOME;                 /* Pointer to HOME ENV variable */
extern char*   LOGNAME;              /* Pointer to LOGNAME ENV variable */
extern char    line[LINE_MAX];       /* Command user input */
extern char    command[64];          /* Command string */
extern char    head[512], tail[512]; /* Child process vars */

char* searchPath(char* env[]);
char* searchHome(char* env[]);

int main (int argc, char* args[], char* env[])
{
    char *token;                /* Pointer for tokens during toeknizing */
    char tmpString[LINE_MAX];   /* Temp string for tokenizing */
    int  pid, status, count = 0;           /* Process id and status */

    system("clear");
    __init__(env);
    char *path = searchPath(env);
    printf("***************** Welcome to rtsh *****************\n");
    printf("1. show PATH:\n%s\n\n", path);

    char *dir;
    dir = 5 + strtok(path, ":");
    printf("2. decompose PATH into dir strings:\n%s ", dir);

    while(dir = strtok(NULL, ":"))
    {
        count++;
        printf("%s ", dir);
    }

    char *home = 5 + searchHome(env);
    printf("\n\n3. show HOME directory: HOME = %s\n\n", home);

    printf("4. ***************** rtsh processing loop *****************\n");


    while (1)
    {
        // Print shell prompt and wait for command
        printf("rtsh %% : ");
        fgets(line, LINE_MAX - 1, stdin);

        // Empty string will loop back to show command prompt
        short len = strlen(line);
        if (0 == len)
        {
            continue;
        }

        // Kill the newline character at end of string
        line[len - 1] = 0;

        // Extract command from user input line
        strcpy(tmpString, line);
        token = strtok(tmpString, " ");

        // When user inputs whitespaces, it loops back to show command prompt
        if (NULL == token)
        {
            continue;
        }
        strcpy(command, token);

        // Exits the program (Simple Command)
        if (0 == strcmp(command, "exit"))
        {
            fprintf(stderr, "%s %d exits\n", SHELL_NAME, getpid());
            exit(0);
        }
        // Change directory (Simple Command)
        else if (0 == strcmp(command, "cd"))
        {
            token = strtok(NULL, " ");
            // Home directory
            if (NULL == token)
            {
                chdir(HOME);
                fprintf(stderr, "%d cd to HOME\n", getpid());
            }
            // Relative/Absolute directory
            else
            {
                if (0 == chdir(token))
                {
                    fprintf(stderr, "%d cd to %s OK\n", getpid(), token);
                }
                else
                {
                    fprintf(stderr, "%d cd to %s FAILED!\n", getpid(), token);
                }
            }
        }
        // Other Commands
        else
        {
            // fork a child process
            pid = fork();

            // Ensure the it was able to fork
            if (pid < 0)
            {
                fprintf(stderr, "Fork failed!");
                exit(-1);
            }

            // Parent executes
            if (pid)
            {
                fprintf(stderr, "parent %s %d forks a child process %d\n", SHELL_NAME, getpid(), pid);
                fprintf(stderr, "parent %s %d waits\n", SHELL_NAME, getpid());
                pid = wait(&status);
                fprintf(stderr, "child %s %d died : exit status = %04x\n", SHELL_NAME, pid, status);
            }
            // Child executes
            else
            {
                fprintf(stderr, "PROC %d line=%s\n", getpid(), line);

                // Setting initial values for head and tail
                strcpy(head, line);
                strcpy(tail, "");
                executeCommand(env);

                exit(0);
            }
        }

    }

    return 0;
}
char* searchPath(char* env[])
{
    while(*env)
    {
        char *str = *env;
        if(strncmp(str,"PATH", 4) == 0)
        {
            return str;
        }
        *env++;
    }
    return NULL;
}

char* searchHome(char* env[])
{
    while(*env)
    {
        char *str = *env;
        if(strncmp(str,"HOME", 4) == 0)
        {
            return str;
        }
        *env++;
    }
    return NULL;
}
