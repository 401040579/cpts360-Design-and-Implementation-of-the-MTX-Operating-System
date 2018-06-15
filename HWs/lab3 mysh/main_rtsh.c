#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
char* searchPath(char* env[]);
char* searchHome(char* env[]);
void soSomething(char* env[]);

char **myargv;
char inputcp[64];
char *cmd;
int count = 0; /*count how many dir in env*/
char *head, *tail;
char *list_dir[64];

main(int argc, char* argv[], char* env[])
{
    /*
        int i = 0;
        while(*env)
        {
            printf("env[%d]=%s\n", i, *env);
            *env++;
            i++;
        }
    */
    char *path = searchPath(env);
    printf("***************** Welcome to rtsh *****************\n");
    printf("1. show PATH:\n%s\n\n", path);

    char *dir;
    dir = 5 + strtok(path, ":");
    strcpy(&list_dir[0], &dir);
    printf("2. decompose PATH into dir strings:\n%s ", dir);

    while(dir = strtok(NULL, ":")) //copy dir to list_dir[]
    {
        count++;
        printf("%s ", dir);
        strcpy(&list_dir[count], &dir);
    }

    char *home = 5 + searchHome(env);
    printf("\n\n3. show HOME directory: HOME = %s\n\n", home);

    printf("4. ***************** rtsh processing loop *****************\n");

    while(1)
    {
        int pid, status;
        char input[64];
        printf("rtsh %% : ");
        fgets(input, sizeof(input), stdin);
        input[strlen(input)-1]=0; //delete enter key
        strcpy(inputcp,input);
        if(strlen(input) == 0)
        {
            printf("line empty.\n");
            continue;
        }


        cmd = strtok(input, " ");
        char *tempPath = strtok(NULL, " ");

        // cd
        if(strcmp(cmd, "cd") == 0)
        {
            printf("cd cmd.\n");
            if(tempPath == NULL)
            {
                chdir(home);
                printf("HOME = %s\n",home);
                printf("%d cd to HOME\n", getpid());
            }
            else
            {
                if(chdir(tempPath) == 0)
                {
                    printf("PROC %d cd to %s OK\n", getpid(), tempPath);
                }
                else
                {
                    printf("PROC %d cd to %s FAILED!\n", getpid(), tempPath);
                }
            }
            continue;
        }

        // exit
        if(strcmp(cmd, "exit") == 0)
        {
            printf("rtsh PROC %d exits\n", getpid());
            exit(1);
        }

        printf("other cmd\n");

        //3.1 fork a child process
        pid = fork(); //parent returns child pid, child returns 0.

        if(pid < 0)
        {
            perror("fork failed");
            exit(-1);
        }

        if(pid) // parent
        {
            printf("parent rtsh %d forks a child process %d\n", getpid(), pid);
            printf("parent rtsh %d waits\n", getpid());
            //3.2 wait for the child to terminate
            pid = wait(&status);
            //3.3 print child's exit status code
            printf("child rtsh PROC %d died : exit status = %04x\n", pid, status);
        }
        else // child
        {
            printf("PROC %d: line=%s\n",getpid(),inputcp);
            printf("PROC %d: do_command: line=%s\n",getpid(),inputcp);
            doSomething(env);

            exit(0);
        }
    }
}

void doSomething(char* env[])
{
    char *partOfInputcp;
    int i = 0,hasPipe=0, r;
    int success = 0;
    char inputcp2[64];
    char *tok;
    int pid;
    int pd[2];
    strcpy(inputcp2, inputcp);
    printf("inputcp2: %s\n", inputcp2);

    head = strtok(inputcp2,"|");
    tail = strtok(NULL,"");
    if(tail == NULL)
    {
        printf("no pipe\n");
        hasPipe = 0;
    }
    else
    {
        printf("has pipe\n");
        hasPipe = 1;
    }

    if(hasPipe)
    {
        r = pipe(pd);
        printf("pd[0]=%d\n", pd[0]);
        printf("pd[1]=%d\n", pd[1]);
        pid = fork(); // fork a child pid
        if(pid<0)
        {
            printf("fork failed.\n");
            exit(-1);
        }
    }

    if(pid)
    {
        if (hasPipe)
        {
            close(pd[1]);
            close(0);
            dup(pd[0]);

        }

        printf("head=%s tail=%s\n", head, tail);
        //partOfInputcp = strtok(inputcp, " ");
        partOfInputcp = strtok(head, " ");
        myargv = calloc(8, sizeof(char*));
        myargv[0] = partOfInputcp;
        printf("myargv[%d]:  %s\n", i, partOfInputcp);
        while(partOfInputcp = strtok(NULL, " "))
        {
            i++;
            printf("myargv[%d]:  %s\n", i, partOfInputcp);
            myargv[i] = partOfInputcp;
        }
        printf("PROC %d tries %s in each PATH dir\n", getpid(), myargv[0]);

        for(int i = 0; i < count + 1; i++)
        {
            char filename[64];
            strcpy(filename, list_dir[i]);
            strcat(filename, "/");
            strcat(filename, myargv[0]);
            printf("i=%d   cmd=%s\n", i, filename);

            if(execve(filename, myargv, env) != -1)
            {
                success = 1;
                break;
            }
        }
        if(!success) printf("invalid command %s\n", myargv[0]);
    }
    else
    {
        close(pd[0]);
        close(1);
        dup(pd[1]);
        //doSomething(env);
    }

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
