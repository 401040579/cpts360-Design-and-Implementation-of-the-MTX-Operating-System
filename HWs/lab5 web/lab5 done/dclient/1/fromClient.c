
     CS360 Lab Assignment #5 : File Operations across Networks
               
                     WORK IN 2-PERSON TEAMS
                   DUE & DENO : Week of 3-6-2017

A. OBJECTIVES:
   Network Programming using TCP.
   Unix system calls for file operations.

B. TCP/IP Program:
   samples/LAB5/: server.c and client.c files

   The sample solution can do a lot of user commands; both local and remote. 

C. REQUIREMENTS:

   Modify the server.c and client.c programs to do the following:

     Client : input a command, which is one of the following:
  
              get   pathname       (cp pathname to client side)
              put   pathname       (cp pathanme to server side)

              quit                 (Client exits)
           -----------------
           send command to Server.
           receive reply AND results from Server. Display the results

     Server:
             get a command from Client;
             perform the command;
             send results to Client;

    *************************************************************
    *  OUTPUTS REQUIREMENTS: CONSULT THE POSTED SAMPLE SOLUTION *
    *************************************************************

C. HELP Hints:


(1). Make each command a fixed-length string, e.g. of MAX=256 bytes.
     REASON: a TCP socket contains a "stream" of data. Each read operation
             reads whatever is available in the socket. Using fixed-length 
             items simplifies reading individual command strings.

(2). Use sscanf()  to read  integers, strings, etc. from a buf[ ] area.
     Use sprintf() to write integers, strings, etc. to   a buf[ ] area.


(3). Assume get filname, which downloads a file from the server:

            CLIENT                             SERVER
   -------------------------------    -------------------------------
  send request (get filename)  ====>  stat filename to get file type AND SIZE
                                           file type MUST be a REGular file  
  wait for reply               <===   send SIZE=xxxx or BAD 
  if (BAD): next input;               if (BAD): next command
  ====================================================================
  count = 0;                         
  open filename for WRITE             open filename for READ
  while(count < SIZE){                while(n=read(fd, buf, MAX)){
     n = read(socket,buf,MAX); <=====   send n bytes from buf
     count += n;
     write n bytes to file;
  }                                   }      
  close file;                         close file;
  /*******************************************************************/


(4). You figure out HOW TO put filname,which uploads a file to the server.


                    Sample Solutions
             samples/LAB5/ : server  and client

