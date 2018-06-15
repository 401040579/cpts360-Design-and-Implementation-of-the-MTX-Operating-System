// The echo client client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#define MAX 256

// Define variables
struct hostent *hp;
struct sockaddr_in  server_addr;

int server_sock, r;
int SERVER_IP, SERVER_PORT;

char *cmds[] = {"get", "put", 0};
int(*fptr[])(char *) = { (int(*)())get, put};
// clinet initialization code

int client_init(char *argv[])
{
  printf("======= clinet init ==========\n");

  printf("1 : get server info\n");
  hp = gethostbyname(argv[1]);
  if (hp == 0) {
    printf("unknown host %s\n", argv[1]);
    exit(1);
  }

  SERVER_IP   = *(long *)hp->h_addr;
  SERVER_PORT = atoi(argv[2]);

  printf("2 : create a TCP socket\n");
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock < 0) {
    printf("socket call failed\n");
    exit(2);
  }

  printf("3 : fill server_addr with server's IP and PORT#\n");
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = SERVER_IP;
  server_addr.sin_port = htons(SERVER_PORT);

  // Connect to server
  printf("4 : connecting to server ....\n");
  r = connect(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (r < 0) {
    printf("connect failed\n");
    exit(1);
  }

  printf("5 : connected OK to \007\n");
  printf("---------------------------------------------------------\n");
  printf("hostname=%s  IP=%s  PORT=%d\n",
         hp->h_name, inet_ntoa(SERVER_IP), SERVER_PORT);
  printf("---------------------------------------------------------\n");

  printf("========= init done ==========\n");
}

main(int argc, char *argv[ ])
{
  int n;
  char line[MAX], cmd[MAX], filename[MAX];
  char cwd[128];
  char buf[1024];
  getcwd(cwd, 128);
  chroot(cwd);
  chdir("/");
  getcwd(cwd, 128);
  printf("cwd: %s\n", cwd);
  if (argc < 3) {
    printf("Usage : client ServerName SeverPort\n");
    exit(1);
  }

  client_init(argv);
  // sock <---> server
  printf("********  processing loop  *********\n");
  while (1) {
    printf("input a command : ");
    bzero(line, MAX); bzero(cmd, MAX); bzero(filename, MAX); // zero out line[ ]
    bzero(buf, 1024);
    fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

    line[strlen(line) - 1] = 0;      // kill \n at end
    if (line[0] == 0)                // exit if NULL line
      exit(0);

    /*printf a filename from client*/
    //char cmd[64],filename[64];
    //sscanf(line, "%s %s", &cmd, &filename);
    //printf("filename=%s\n", filename);

    // Send ENTIRE line to server
    write(server_sock, line, MAX);
    //printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

    // Read a line from sock and show it
    read(server_sock, filename, MAX);
    printf("filename=%s\n", filename);
    //printf("client: read  n=%d bytes; echo=(%s)\n", n, ans);

    //start
    sscanf(line, "%s %s", &cmd, &filename);

    if (strcmp(cmd, "get") == 0)
    {
      //open file
      read(server_sock, buf, 1024);
      if (strcmp(buf, "OK") != 0) {printf("%s", buf); continue;}


      //stat file
      read(server_sock, buf, 1024);
      if (strcmp(buf, "OK") != 0) {printf("%s", buf); continue;}


      //read SIZE=xxxx or BAD
      read(server_sock, buf, 1024);
      if (strcmp(buf, "BAD") == 0) continue;
      int size;
      sscanf(buf, "SIZE %d", &size);
      printf("exorecting %d bytes\n", size);
      //write to file
      int count = 0;
      int gd, n;
      gd = open(filename, O_WRONLY | O_CREAT, 0777);
      while (count < size)
      {
        n = read(server_sock, buf, 1024);
        count += n;
        printf("n=%d total=%d\n", n, count);
        write(gd, buf, n);
      }
      printf("received %d bytes\n", count);
      close(gd);
    }
    else if (strcmp(cmd, "put") == 0)
    {
      int n, total = 0;
      int buf[1024];
      bzero(buf, 1024);
      int fd;
      fd = open(filename, O_RDONLY);
      if (fd < 0)
      {
        sprintf(buf, "client: can't open %s for WRITE\n", filename);
        write(server_sock, buf, 1024);
        printf("client: can't open %s for WRITE\n", filename);
      }
      else
      {
        sprintf(buf, "OK");
        write(server_sock, buf, 1024);

        struct stat fstat, *sp;
        int r;
        sp = &fstat;

        //stat file
          if ( (r = lstat(filename, sp)) < 0)
          {
            sprintf(buf, "can't stat %s\n", filename);
            write(server_sock, buf, 1024);
            printf("can't stat %s\n", filename);
          }
          else
          {
            sprintf(buf, "OK");
            write(server_sock, buf, 1024);

            //write SIZE=xxxx or BAD
            if (S_ISREG(sp->st_mode))
            {
              sprintf(buf, "SIZE %d", sp->st_size);
              write(server_sock, buf, 1024);
            }
            else
            {
              sprintf(buf, "BAD");
              write(server_sock, buf, 1024);
              printf("it is not REGular file\n");
            }


            if (strcmp(buf, "BAD") == 0) continue;
            //int count = 0;
            //int fd, gd;
            //fd = open(filename, O_RDONLY);
            int count = 0;
            while (n = read(fd, buf, 1024))
            {
              write(server_sock, buf, n);
              count += n;
              printf("n=%d total=%d\n", n, count);
            }
            printf("sent %d bytes\n", count);
            close(fd);
          }
      }
    }
    else if (strcmp(cmd, "quit") == 0)
    {
      exit(1);
    }
  }
}


