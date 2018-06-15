#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h>

//#include <unistd.h>
#include "type.h"

#define SIZEOFPATH 100
MINODE minode[NMINODE];        // global minode[ ] array
MINODE *root;                  // root pointer: the /
PROC   proc[NPROC], *running;  // PROC; using only proc[0]
OFT    oft[NOFT];

int fd, dev;                               // file descriptor or dev
int nblocks, ninodes, bmap, imap, iblock;  // FS constants
int i;

//char line[128], cmd[64], pathname[64];
char line[192], cmd[64], pathname[64], newFileName[64];
char buf[BLKSIZE];              // define buf1[ ], buf2[ ], etc. as you need

int ino, bno;
char *name[SIZEOFPATH];
int n;

int dev;
int nblocks;
int ninodes;
int bmap;
int imap;
int iblock;

#include "iget_iput_getino.c"
#include "ls_cd_pwd_quit.c"
#include "alloc_dealloc.c"
#include "mkdir_creat.c"
#include "rmdir.c"
#include "link_unlink_symlink.c"
#include "open_close.c"
#include "read.c"
#include "write.c"
#include "touch_chmod.c"
//#include "iget_iput_getino.c"  // YOUR iget_iput_getino.c file with
// get_block/put_block, tst/set/clr bit functions
// global variables

//(3). init();         // write C code
/*
int init()
{
  WRITE C code to initialize

  (1).All minode's refCount = 0;
  (2).proc[0]'s pid = 1, uid = 0, cwd = 0, fd[ ] = 0;
  proc[1]'s pid = 2, uid = 1, cwd = 0, fd[ ] = 0;
}
*/
int init() // Initialize data structures of LEVEL-1:
{
	/*
	(1). 2 PROCs, P0 with uid=0, P1 with uid=1, all PROC.cwd = 0;
	(2). MINODE minode[100]; all with refCount=0;
	(3). MINODE *root = 0;
	*/
	MINODE *mip;

	printf("init()\n");
	for (int i = 0; i < NPROC; i++)
	{
		running = &proc[i];
		running->uid = i;
		running->gid = i;
		running->pid = i + 1;
		running->cwd = 0;
		for (int j = 0; j < NFD; j++)
		{
			running->fd[j] = 0;
		}
	}

	for (int i = 0; i < NMINODE; i++)
	{
		mip = &minode[i];
		mip->dev = mip->ino = 0;
		mip->refCount = 0;
		mip->mounted = 0;
		mip->mptr = 0;
	}

	root = 0;
}

//(4). mount_root();   // write C code
// load root INODE and set root pointer to it
int mount_root()
{
	printf("mount_root()\n");
	root = iget(dev, 2); // Do you understand this?
	//root will be inode#2 from the dev
}


char *disk = "mydisk";
//******************************** main **************************************
main(int argc, char *argv[ ])   // run as a.out [diskname]
{

	if (argc > 1)
		disk = argv[1];

	if ((dev = fd = open(disk, O_RDWR)) < 0) {
		printf("open %s failed\n", disk);
		exit(1);
	}
	//print fd or dev to see its value!!!
	printf("fd = dev = %d\n", fd);

	/*
	(1). printf("checking EXT2 FS\n");

	     Write C code to check EXT2 FS; if not EXT2 FS: exit

	     get ninodes, nblocks (and print their values)
	*/
	get_block(fd, 1, buf);
	sp = (SUPER *)buf;

	printf("check ext2 FS on %s :", disk);
	if (sp->s_magic != 0xEF53) {
		printf("NOT an EXT2 FS\n");
		exit(2);
	}
	else
		printf("YES\n");

	ninodes = sp->s_inodes_count;
	printf("ninodes = %d\n", ninodes);

	nblocks = sp->s_blocks_count;
	printf("nblocks = %d\n", nblocks);

	//(2). Read GD block to get bmap, imap, iblock ( and print their values)
	get_block(fd, 2, buf);
	gp = (GD *)buf;

	//bmap
	bmap = gp->bg_block_bitmap;
	printf("bmap = %d ", bmap);

	//imap
	imap = gp->bg_inode_bitmap;
	printf("imap = %d ", imap);

	//iblock
	iblock = gp->bg_inode_table;
	printf("iblock = %d\n", iblock);

	//(3)
	init();

	//(4)
	mount_root(); //root->refCount = 1;

	printf("root refCount = %d\n", root->refCount);

	//(5)
	printf("creating P0 as running process\n");
	/*
	WRITE C code to do these:
	  set running pointer to point at proc[0];
	  set running's cwd   to point at / in memory;
	*/
	running = &proc[0];
	running->cwd = iget(dev, 2);
	printf("root refCount = %d\n", root->refCount);



	//(6)
	while (1) {
		printf("input command LEVEL-1: [ls|cd|pwd|quit|mkdir|creat|rmdir|link|unlink|symlink]\n");
		printf("input command LEVEL-2: |lseek|pfd|open|close|lseek|read|cat|write|cp|mv] ");

		//use fgets() to get user inputs into line[128]
		fgets(line, 196, stdin);
		//kill the \r at end
		line[strlen(line) - 1] = 0;

		//if (line is an empty string) // if user entered only \r
		//continue;
		if (line[0] == 0)
			continue;

		pathname[0] = 0;

		newFileName[0] = 0;

		//Use sscanf() to extract cmd[ ] and pathname[] from line[128]
		sscanf(line, "%s %s %s", cmd, pathname, newFileName);
		printf("cmd=%s pathname=%s newFileName=%s\n", cmd, pathname, newFileName);

		// execute the cmd
		if (strcmp(cmd, "ls") == 0)
			my_ls(pathname);
		if (strcmp(cmd, "cd") == 0)
			my_chdir(pathname);
		if (strcmp(cmd, "pwd") == 0)
			my_pwd(running->cwd);
		if (strcmp(cmd, "quit") == 0)
			my_quit();
		if (strcmp(cmd, "mkdir") == 0)
			make_dir();
		if (strcmp(cmd, "creat") == 0)
			creat_file();
		if (strcmp(cmd, "rmdir") == 0)
			my_rmdir();
		if (strcmp(cmd, "link") == 0)
			mylink();
		if (strcmp(cmd, "unlink") == 0)
			my_unlink();
		if (strcmp(cmd, "rm") == 0)
			my_unlink();
		if (strcmp(cmd, "symlink") == 0)
			my_symlink();
		if (strcmp(cmd, "touch") == 0)
			my_touch();
		if (strcmp(cmd, "chmod") == 0)
			my_chmod();

		if (strcmp(cmd, "open") == 0) {
			open_file();
		}
		if (strcmp(cmd, "close") == 0) {
			my_close();
		}
		if (strcmp(cmd, "lseek") == 0) {
			lseek_file();
		}
		if (strcmp(cmd, "pfd") == 0) {
			pfd();
		}
		if (strcmp(cmd, "read") == 0) {
			read_file();
		}
		if (strcmp(cmd, "cat") == 0) {
			cat_file();
		}
		if (strcmp(cmd, "write") == 0) {
			write_file();
		}
		if (strcmp(cmd, "cp") == 0) {
			mycp();
		}
		if (strcmp(cmd, "mv") == 0) {
			mymv();
		}
	}
}