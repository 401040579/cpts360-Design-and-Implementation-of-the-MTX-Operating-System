#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX 10000
typedef struct {
	char *name;
	char *value;
} ENTRY;

ENTRY entry[MAX];

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int my_ls(char *fname)
{
	struct stat fstat, *sp;
	int r, i, readlink_res;
	char ftime[64];
	char buf[MAX];
	sp = &fstat;

	if ( (r = lstat(fname, &fstat)) < 0)
	{
		printf("can't stat %s<p>", fname);
		return;
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
		for (i = 0; i < readlink_res; i++)
		{
			putchar(buf[i]);
		}

	}
	printf("<p>");
}

main(int argc, char *argv[])
{
	int i, m, r;
	char cwd[128];

	m = getinputs();    // get user inputs name=value into entry[ ]
	getcwd(cwd, 128);   // get CWD pathname

	printf("Content-type: text/html\n\n");
	printf("<p>pid=%d uid=%d cwd=%s\n", getpid(), getuid(), cwd);

	printf("<H1>Echo Your Inputs</H1>");
	printf("You submitted the following name/value pairs:<p>");

	for (i = 0; i <= m; i++)
		printf("%s = %s<p>", entry[i].name, entry[i].value);
	printf("<p>");


	/*****************************************************************
	 Write YOUR C code here to processs the command
	       mkdir dirname
	       rmdir dirname
	       rm    filename
	       cat   filename
	       cp    file1 file2
	       ls    [dirname] <== ls CWD if no dirname
	*****************************************************************/
	/*
	printf("entry[0].name = %s, entry[0].value = %s<p>", entry[0].name, entry[0].value);
	printf("entry[1].name = %s, entry[1].value = %s<p>", entry[1].name, entry[1].value);
	printf("entry[2].name = %s, entry[2].value = %s<p>", entry[2].name, entry[2].value);
	*/
	printf("<H1>Processing Results</H1>");

	if (strcmp(entry[0].value, "mkdir") == 0)
	{
		if (mkdir(entry[1].value, 0777) >= 0)
			printf("%s %s success<p>", entry[0].value, entry[1].value);
		else
			printf("%s %s failed<p>", entry[0].value, entry[1].value);
	}
	else if (strcmp(entry[0].value, "rmdir") == 0)
	{
		if (rmdir(entry[1].value) >= 0)
			printf("%s %s success<p>", entry[0].value, entry[1].value);
		else
			printf("%s %s failed<p>", entry[0].value, entry[1].value);
	}
	else if (strcmp(entry[0].value, "rm") == 0)
	{
		if (unlink(entry[1].value) >= 0)
			printf("%s %s success<p>", entry[0].value, entry[1].value);
		else
			printf("%s %s failed<p>", entry[0].value, entry[1].value);
	}
	else if (strcmp(entry[0].value, "cat") == 0)
	{
		if (1) {
			int fd;
			int i, n;
			char buf[1024];

			if (strcmp(entry[1].value, "") == 0)
			{
				printf("need a filename<p>");
				printf("%s %s failed<p>", entry[0].value, entry[1].value);
			}
			else
			{
				fd = open(entry[1].value, O_RDONLY);

				if (fd < 0)
				{
					printf("%s %s failed<p>", entry[0].value, entry[1].value);
				}
				else
				{
					while (n = read(fd, buf, 1024))
					{
						for (i = 0; i < n; i++)
						{
							if (buf[i] == '\n') printf("<p>");
							else if (buf[i] == '<') printf("&lt");
							else if (buf[i] == '>') printf("&gt");

							else putchar(buf[i]);
						}
					}
					printf("<p>");
					printf("%s %s success<p>", entry[0].value, entry[1].value);
				}
			}
		}/*
  	else
  	{
  		FILE *fp;
  		int c;
  		if(entry[1].value == NULL)
  			printf("filename1 empty<p>");

  		fp = fopen(entry[1].value, "r");

  		if(fp == 0)
  			printf("%s %s fail<p>", entry[0].value, entry[1].value);

  		while((c = fgetc(fp))!=EOF)
  		{
  			putchar(c);
  		}
  	}*/
	}
	else if (strcmp(entry[0].value, "cp") == 0)
	{
		int n, total = 0;
		int fd, gd;
		char buf[4096];
		if (strcmp(entry[1].value, "") == 0 || strcmp(entry[2].value, "") == 0) printf("filename empty<p>");
		else
		{
			fd = open(entry[1].value, O_RDONLY);

			if (fd < 0) printf("open file failed<p>");
			else
			{
				gd = open(entry[2].value, O_WRONLY | O_CREAT, 0777);
				while (n = read(fd, buf, 4096))
				{
					write(gd, buf, n);
					total += n;
				}
				printf("<p>");
				printf("%s %s success<p>", entry[0].value, entry[1].value);
				close(fd); close(gd);
			}

		}
	}
	else if (strcmp(entry[0].value, "ls") == 0)
	{
		struct stat mystat, *sp;
		struct dirent *dirp;
		DIR *dp;
		char dName[256];
		char pathname[256];

		strcpy(pathname, cwd);

		if ((entry[1].value)[0] == '/')
		{
			strcpy(pathname, entry[1].value);
			if (pathname[strlen(pathname) - 1] != '/') strcat(pathname, "/");
			printf("ls %s", pathname);
		}
		else if (strcmp(entry[1].value, "") != 0)
		{
			strcat(pathname, "/");
			strcat(pathname, entry[1].value);
			strcat(pathname, "/");
			printf("ls %s", pathname);
		}
		else
		{
			strcat(pathname, "/");
			printf("ls %s", pathname);
		}

		printf("<p>");
		dp = opendir(pathname);
		if (dp != NULL)
		{
			while (dirp = readdir(dp))
			{
				char tp_path[256];
				strcpy(tp_path, pathname);
				strcpy(dName, dirp->d_name);
				if (dName[0] == '.') continue;
				strcat(pathname, dName);
				//printf("%s<p>", pathname);
				my_ls(pathname);
				strcpy(pathname, tp_path);
			}
			printf("%s %s success<p>", entry[0].value, entry[1].value);
		}
		else
		{
			printf("%s %s failed<p>", entry[0].value, entry[1].value);
		}
	}
	else
	{
		printf("%s %s %s failed<p>", entry[0].value, entry[1].value, entry[2].value);
	}



	// create a FORM webpage for user to submit again
	printf("</title>");
	printf("</head>");
	printf("<body bgcolor=\"#aae2b2\" link=\"#330033\" leftmargin=8 topmargin=8");
	printf("<p>------------------ DO IT AGAIN ----------------\n");

	printf("<FORM METHOD=\"POST\" ACTION=\"http://cs360.eecs.wsu.edu/~taoran/cgi-bin/mycgi\">");

	//------ NOTE : CHANGE ACTION to YOUR login name ----------------------------
	//printf("<FORM METHOD=\"POST\" ACTION=\"http://cs360.eecs.wsu.edu/~YOURNAME/cgi-bin/mycgi\">");

	printf("Enter command : <INPUT NAME=\"command\"> <P>");
	printf("Enter filename1: <INPUT NAME=\"filename1\"> <P>");
	printf("Enter filename2: <INPUT NAME=\"filename2\"> <P>");
	printf("Submit command: <INPUT TYPE=\"submit\" VALUE=\"Click to Submit\"><P>");
	printf("</form>");
	printf("------------------------------------------------<p>");

	printf("</body>");
	printf("</html>");
}
