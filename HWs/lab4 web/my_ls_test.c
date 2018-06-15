#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#define MAX 1024

struct stat mystat, *sp;

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int my_ls(char *fname)
{
    struct stat fstat, *sp;
    int r, i, readlink_res;
    char ftime[64];
    char buf[MAX];
    sp = &fstat;
    //printf("name=%s\n", fname); getchar();

    if ( (r = lstat(fname, &fstat)) < 0)
    {
        printf("can't stat %s\n", fname);
        exit(1);
    }

    if ((sp->st_mode & 0xF000) == 0x8000)
        printf("%c", '-');
    if ((sp->st_mode & 0xF000) == 0x4000)
        printf("%c", 'd');
    if ((sp->st_mode & 0xF000) == 0xA000)
        printf("%c", 'l');

    for (i = 8; i >= 0; i--)
    {
        if (sp->st_mode & (1 << i))
            printf("%c", t1[i]);
        else
            printf("%c", t2[i]);
    }

    printf("%4d ", sp->st_nlink);
    printf("%4d ", sp->st_gid);
    printf("%4d ", sp->st_uid);
    printf("%8d ", sp->st_size);

    // print time
    strcpy(ftime, ctime(&sp->st_ctime));
    ftime[strlen(ftime) - 1] = 0;
    printf("%s  ", ftime);

    // print name
    printf("%s", basename(fname));

    // print -> linkname if it's a symbolic file
    if ((sp->st_mode & 0xF000) == 0xA000) // YOU FINISH THIS PART
    {
        // use readlink() SYSCALL to read the linkname
        // printf(" -> %s", linkname);
        printf(" -> ");
        readlink_res = readlink(fname, buf, MAX);
        for (int i = 0; i < readlink_res; i++)
        {
            putchar(buf[i]);
        }

    }
    printf("\n");
}

main(int argc, char *argv[])
{
    struct stat mystat, *sp;
    struct dirent *entry;
    int r;
    char *s;
    char name[1024], cwd[1024];
    DIR *dp;

    s = argv[1];

    if (argc == 1)
        s = "./";

    sp = &mystat;
    if (r = lstat(s, sp) < 0)
    {
        printf("no such file %s\n", s);
        exit(1);
    }
    strcpy(name, s);

    if (s[0] != '/')     // name is relative : get CWD path
    {
        getcwd(cwd, 1024);
        strcpy(name, cwd);
        strcat(name, "/");
        strcat(name, s);
    }
    printf("%s\n", name);

    dp = opendir(name);
    char dName[256];
    while (entry = readdir(dp))
    {
        strcpy(dName, entry->d_name);
        if (dName[0] == '.') continue;
        printf("%s\n", dName);
        my_ls(dName);
    }



    /*
    if (S_ISDIR(sp->st_mode))
        ls_dir(name);
    else
        ls_file(name);
    */
}
