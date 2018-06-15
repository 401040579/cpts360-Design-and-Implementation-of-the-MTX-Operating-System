int tokenize_with_no_printf(char *pathname)
{
	n = 0;
	name[n] = strtok(pathname, "/");
	while (1)
	{
		if (name[n] == NULL) break;
		n++;
		if (n >= SIZEOFPATH) break;
		name[n] = strtok(NULL, "/");
	}
}

int search_with_no_printf(MINODE *mip, char *name) //(mip, name[i])
{
	char lbuf[BLKSIZE];
	char namebuf[256];
	int mblock;
	char *cp;
	//add for loop can do i_block[0-15]

	mblock = mip->INODE.i_block[0];

	if (mblock == 0) return 0;

	get_block(mip->dev, mblock, lbuf);
	cp = lbuf;
	dp = (DIR *)lbuf;
	//printf("dp->name = %s", dp->name);
	//printf("cp = %d &lbuf[BLKSIZE] = %d", cp, &lbuf[BLKSIZE]);
	while (cp < &lbuf[BLKSIZE])
	{
		//getchar();
		strncpy(namebuf, dp->name, dp->name_len);
		namebuf[dp->name_len] = 0;

		if (strcmp(name, namebuf) == 0)
			return dp->inode;

		cp += dp->rec_len;
		dp = (DIR*) cp;
		//printf("pause[search]\n");getchar();
	}
	return 0;
}

// search in minode, if found return mip, else allocat a new minode
MINODE *iget_with_no_printf(int dev, int ino)
{
	int i, blk, disp;
	char buf[BLKSIZE];
	MINODE *mip;
	INODE *ip;

	for (i = 0; i < NMINODE; i++) {
		mip = &minode[i];
		if (mip->dev == dev && mip->ino == ino) {
			mip->refCount++;
			//printf("found [dev=%d ino=%d] as minode[%d] in core\n", dev, ino, i);
			return mip;
		}
	}

	for (i = 0; i < NMINODE; i++) {
		mip = &minode[i];
		if (mip->refCount == 0) {
			//printf("allocating NEW minode[%d] for [dev=%d ino=%d]\n", i, dev, ino);
			mip->refCount = 1;
			mip->dev = dev; mip->ino = ino;  // assing to (dev, ino)
			mip->dirty = mip->mounted = mip->mptr = 0; //should be mptr?mountPtr
			// get INODE of ino into buf[ ]
			blk  = (ino - 1) / 8 + iblock; // iblock = Inodes start block #
			disp = (ino - 1) % 8;
			//printf("iget: ino=%d blk=%d disp=%d\n", ino, blk, disp);
			get_block(dev, blk, buf);
			ip = (INODE *)buf + disp;
			// copy INODE to mp->INODE
			mip->INODE = *ip;
			return mip;
		}
	}
	printf("PANIC: no more free minodes\n");
	return 0;
}
int getino_with_no_printf(int *dev, char *pathname)
{
	int i;
	char lbuf[64];
	MINODE *mip;

	//printf("getino: pathname=%s\n", pathname);
	if (strcmp(pathname, "/") == 0)
		return 2;

	if (pathname[0] == '/')
		mip = iget_with_no_printf(*dev, 2);
	else
		mip = iget_with_no_printf(running->cwd->dev, running->cwd->ino);


	strcpy(lbuf, pathname);
	tokenize_with_no_printf(lbuf); // n = number of token strings

	for (i = 0; i < n; i++) {
		//printf("===========================================\n");
		//printf("getino: i=%d name[%d]=%s\n", i, i, name[i]); //used to be kcwname


		ino = search_with_no_printf(mip, name[i]);

		if (ino == 0) {
			iput_with_no_printf(mip);
			//printf("name %s does not exist\n", name[i]);
			return 0;
		}
		iput_with_no_printf(mip);
		mip = iget_with_no_printf(*dev, ino);

	}
	//printf("[***]ino[getino]=%d\n", ino);
	iput_with_no_printf(mip);
	return ino;
}

int iput_with_no_printf(MINODE *mip)
{
	//put mem-inode back

	int blk, disp;
	char buf[BLKSIZE];

	mip->refCount--;

	if (mip->refCount > 0) return;
	if (!mip->dirty)       return;

	/*(3).   write INODE back to disk */

	//printf("[**iput]iput: dev=%d ino=%d\n", mip->dev, mip->ino);

	blk  = (mip->ino - 1) / 8 + iblock;
	disp = (mip->ino - 1) % 8;

	get_block(mip->dev, blk, buf);
	ip = (INODE *)buf + disp;
	*ip = mip->INODE;
	put_block(mip->dev, blk, buf);
}

int my_ls(char *pathname)  // dig out YOUR OLD lab work for ls() code
{
	//no filename runing cwd
	//printf("ls\n");

	/*
	WRITE C code for these :

	determine initial dev :
	if pathname[0] == ' / ': dev = root->dev;
	else                : dev = running->cwd->dev;

	convert pathname to (dev, ino);

	// int inumber = getinode (&dev, ino);
	int inumber = getinulber(dev, pathname);
	return inumber or 0;

	get a MINODE *mip pointing at a minode[ ] with (dev, ino);
	//MINODE *mip = iget(dev,ino);

	if mip->INODE is a file: ls_file(pathname);
	if mip->INODE is a DIR{
	step through DIR entries:
	for each name, ls_file(pathname / name);
	}
	*/

	//start
	MINODE *mip;
	int ino;

	if (pathname[0] == '/') dev = root->dev;
	else  dev = running->cwd->dev;
	printf("[**ls]pathname = %s\n", pathname);


	if (pathname[0] == '\0') //pathname = (null)
		ino = running->cwd->ino;
	else
		ino = getino(&dev, pathname);

	//ino = getino(&dev, pathname);

	if (ino == 0)
	{
		printf("no such file %s\n", pathname);
		return 0;
	}

	printf("[***]ino[ls]=%d\n", ino);
	mip = iget(dev, ino);
	//printf("pause\n"); getchar();
	//printf("mip->INODE->i_mode=%x\n", mip->INODE.i_mode);

	//printf("[**ls]mip->ino=%d\n", mip->ino);

	if (isDir(mip))
	{
		printf("isDir\n");
		ls_dir(pathname, mip);
	}
	else
	{
		printf("not Dir\n");
		//ls_file_old(pathname, mip);
		ls_file(pathname);
		//ls_file(pathname);
	}
	iput(mip);

}

int ls_dir(char *pathname, MINODE *mip) //mip is dir
{
	char lbuf[BLKSIZE];
	char namebuf[256], pathnamecp[64], pathnamecp_or[64];
	int mblock;
	char *cp;

	strcpy(pathnamecp, pathname);
	//printf("[***ls]pathnamecp=%s\n", pathnamecp);

	for (int i = 0; i < 12; i++) {
		mblock = mip->INODE.i_block[i];
		printf("[ls_dir]i_block[%d]=%d\n", i, mblock);
		if (mblock == 0) return 0;
		get_block(fd, mblock, lbuf);
		cp = lbuf;
		DIR *tdp = (DIR *)lbuf;

		//strcat(pathnamecp, "/");

		strcpy(pathnamecp_or, pathnamecp);
		while (cp < &lbuf[BLKSIZE])
		{
			//getchar();
			strncpy(namebuf, tdp->name, tdp->name_len);
			namebuf[tdp->name_len] = 0;
			//printf("[**namebuf]%s\n", namebuf);

			if (pathnamecp[0] != '\0') // if type pathname, add '/'. such as pathname/filename. OW, filename
				strcat(pathnamecp, "/");

			strcat(pathnamecp, namebuf);

			//printf("[**ls]namebuf=%s pathnamecp=%s\n",namebuf, pathnamecp);getchar();
			if (namebuf[0] == '\0')
				ls_file(namebuf);
			else
				ls_file(pathnamecp);
			cp += tdp->rec_len;
			tdp = (DIR*) cp;
			//printf("pause[ls_dir]\n");getchar();
			strcpy(pathnamecp, pathnamecp_or);
		}
	}
	return 0;
}

int ls_file(char *pathname)
{
	int ino;
	MINODE *mip;
	char lbuf[64];
	char time[64];
	char* tfilename, *filename;

	//printf("[**ls_file]pathname=%s\n", pathname);
	ino = getino_with_no_printf(&dev, pathname);
	//printf("[**ls_file]ino=%d\n", ino);
	mip = iget_with_no_printf(dev, ino);
	INODE *tip = &mip->INODE;
	//get ctime
	strcpy(time, ctime(&mip->INODE.i_ctime));
	time[strlen(time) - 1] = '\0';

	//get */*/tfilename
	strcpy(lbuf, pathname);
	tfilename = strtok(lbuf, "/");
	while (filename = strtok(NULL, "/"))
	{
		strcpy(tfilename, filename);
	}

	//printf("[***]ino[ls_file]=%d\n", mip->ino);

	if ((tip->i_mode & 0xF000) == 0x8000)   //0100 = regular file
		printf("%c", '-');
	if ((tip->i_mode & 0xF000) == 0x4000)   //0100 = dir
		printf("%c", 'd');
	if ((tip->i_mode & 0xF000) == 0xA000)   //1010 = link
		printf("%c", 'l');
	char *t1 = "xwrxwrxwr-------";
	char *t2 = "----------------";
	for (i = 8; i >= 0; i--) {                    // wrx permission
		if (tip->i_mode & (1 << i))
			printf("%c", t1[i]);
		else
			printf("%c", t2[i]);
	}

	printf(" %d %d %d %s %d %s",
	       tip->i_links_count, tip->i_uid,
	       tip->i_gid, time, tip->i_size, tfilename);

	if (S_ISLNK(tip->i_mode))
	{
		printf(" -> %s\n", (char *)mip->INODE.i_block);
	}
	else
	{
		printf("\n");
	}
	//getchar();
	iput_with_no_printf(mip);
}

int ls_dir_old(char *pathname, MINODE *mip) //mip is dir
{
	char lbuf[BLKSIZE];
	char namebuf[256];
	int mblock;
	char *cp;

	for (int i = 0; i < 12; i++) {
		mblock = mip->INODE.i_block[i];
		printf("[**ls_dir]:i_block[%d] = %d\n", i, mblock);

		if (mblock == 0) return 0;

		get_block(fd, mblock, lbuf);
		cp = lbuf;
		dp = (DIR *)lbuf;

		while (cp < &lbuf[BLKSIZE])
		{
			//getchar();
			strncpy(namebuf, dp->name, dp->name_len);
			namebuf[dp->name_len] = 0;

			mip = iget_with_no_printf(dev, dp->inode);
			//mip = iget(dev, dp->inode);
			ls_file_old(namebuf, mip);
			cp += dp->rec_len;
			dp = (DIR*) cp;
			//printf("pause[ls_dir]\n");getchar();
		}
	}
	return 0;
}

int ls_file_old(char *pathname, MINODE *mip) //pathname is a file name
{
	char time[64], pathbuf[64];
	char* tfilename, *filename;
	strcpy(time, ctime(&mip->INODE.i_ctime));
	time[strlen(time) - 1] = '\0';



	strcpy(pathbuf, pathname);
	//printf("[***]pathbuf=%s\n", pathbuf);
	//printf("[**ls_file]pathbuf=%s\n", pathbuf);
	tfilename = strtok(pathbuf, "/");
	//printf("[***]strtok[ls_file]=%s\n", filename);
	//printf("[**ls_file]tfilename=%s\n", tfilename);
	while (filename = strtok(NULL, "/"))
	{
		//printf("[***]strtok[ls_file]=%s\n", filename);
		strcpy(tfilename, filename);
	}


	//printf("[***]ino[ls_file]=%d\n", mip->ino);

	printf("%x %d %d %d %s %d %s\n",
	       mip->INODE.i_mode, mip->INODE.i_links_count, mip->INODE.i_uid,
	       mip->INODE.i_uid, time, mip->INODE.i_size, tfilename);

}

int isDir(MINODE *mip)
{
	char imode[16]; char mode[4];
	//printf("mip->INODE->i_mode=%x", mip->INODE.i_mode);
	get_imode(mip->INODE.i_mode, imode);
	//printf("pause\n"); getchar();
	strncpy(mode, imode, 4); mode[4] = '\0';
	if (strcmp(mode, "0100") == 0)
	{
		return 1;
	}
	return 0;
}

int get_imode(int n, char imode[])
{
	for (int i = sizeof(short) * 8 - 1; i >= 0; i--)
	{
		if ((1 << i) & n) {
			//putchar('1');
			imode[15 - i] = '1';
		}
		else {
			//putchar('0');
			imode[15 - i] = '0';
		}
	}
}

int my_pwd(MINODE *mip)
{
	MINODE *tmip;
	char tempPath[64], tempName[64];
	printf("pwd\n");
	//save string locally.
	strcpy(tempPath, "");
	if (mip == root)
	{
		//iput(mip);
		strcat(tempPath, "/");
		printf("%s\n", tempPath);
		return 1;
	}
	int ino = mip->ino; // b's ino
	int upper_ino = search(mip, ".."); //a's ino
	while (1) {
		printf("[**pwd]ino=%d upper_ino=%d\n", ino, upper_ino); //getchar();

		if (upper_ino == ino) break;

		char lbuf[1024];
		char namebuf[256];
		char *cp;

		tmip = iget(running->cwd->dev, upper_ino);

		int bno = tmip->INODE.i_block[0];
		//printf("bno = %d",bno);

		get_block(tmip->dev, bno, lbuf);
		

		cp = lbuf;
		dp = (DIR *)lbuf;

		//printf("dp->name = %s", dp->name);
		//printf("cp = %d &lbuf[BLKSIZE] = %d", cp, &lbuf[BLKSIZE]);
		printf("\ninode rec_len name_len name\n");
		while (cp < &lbuf[BLKSIZE])
		{
			//getchar();
			strncpy(namebuf, dp->name, dp->name_len);
			namebuf[dp->name_len] = 0;
			printf("%5d %7d %8d %4s\n",
			       dp->inode, dp->rec_len, dp->name_len, namebuf);
			//getchar();
			if (dp->inode == ino)
			{
				printf("namebuf=%s\n", namebuf);
				break;
			}
			cp += dp->rec_len;
			dp = (DIR*) cp;
		}

		strcat(tempPath, "/");
		strcat(tempPath, namebuf);

		ino = tmip->ino;
		upper_ino = search(tmip, "..");
		iput(tmip);
	}
	printf("tempPath=%s\n", tempPath); //getchar();
	print_reversePathname(tempPath);
	return 0;

	//search .. for inode.

	//printf("[**pwd]ino=%d\n", ino);

	//return 0;
}

int print_reversePathname(char *Path)
{
	char tempPath[64];
	int i = 0;
	char *tempName[64];

	strcpy(tempPath, Path);
	tempName[i] = strtok(tempPath, "/");
	while (1)
	{
		if (tempName[i] == NULL) break;
		i++;
		if (i >= SIZEOFPATH) break;
		tempName[i] = strtok(NULL, "/");
	}

	for (int j = i - 1; j >= 0; j--)
	{
		printf("/%s", tempName[j]);
	}
	printf("\n");
}

int my_chdir(char *pathname)
{

	//printf("chdir\n");
	int ino, dev;
	//determine initial dev
	if (pathname[0] == '/') dev = root->dev;
	else  dev = running->cwd->dev;

	//convert pathname to (dev, ino);
	if (pathname[0] == '\0') //pathname = (null)
		ino = running->cwd->ino;
	else
		ino = getino(&dev, pathname);
	//ino = getino(&dev, pathname);

	if (ino == 0)
	{
		printf("no such file %s\n", pathname);
		return 0;
	}

	//get a MINODE *mip pointing at (dev, ino) in a minode[ ];
	MINODE *mip = iget(dev, ino);
	//printf("[cd] mip->refCount=%d\n", mip->refCount);getchar();
	//printf("pause\n"); getchar();
	//printf("mip->INODE->i_mode=%x\n", mip->INODE.i_mode);

	/*
	if mip->INODE is a DIR{
	  dispose of OLD cwd; iput(runing->cwd);
	  set cwd to mip; runing->cwd = mip;
	  }
	*/

	printf("S_ISLNK(mip->INODE.i_mode)=%d\n", S_ISLNK(mip->INODE.i_mode));

	if (S_ISDIR(mip->INODE.i_mode))
	{
		printf("isDir\n");
		iput(running->cwd);
		running->cwd = mip;
	}
	else if (S_ISLNK(mip->INODE.i_mode))
	{
		int ino = getino(&dev, (char *)mip->INODE.i_block);
		MINODE *tmip = iget(dev, ino);

		if (S_ISDIR(tmip->INODE.i_mode))
		{
			printf("isDir\n");
			iput(running->cwd);
			running->cwd = tmip;
		}
		else
		{
			printf("this symlink is not dir\n");
		}
		//iput(tmip); dont iput it
		iput(mip);

	}
	else
	{
		//if mip->INODE is NOT a DIR: reject and print error message;
		printf("not Dir\n");
		//ls_file(pathname);
		iput(mip);
	}
	//printf("[cd] mip->refCount=%d\n", mip->refCount);getchar();
	//iput(mip); dont iput it
}
/*
int pwd(running->cwd)//: YOU WRITE CODE FOR THIS ONE!!!
{

}
*/
int my_quit()
{
	/*
	for each minode[ ] do {
	  if  minode[ ]'s refCount != 0:
	      write its INODE back to disk;
	}
	exit(0);  // terminate program
	*/

	int i;
	MINODE *mip;
	for (i = 0; i < NMINODE; i++)
	{
		mip = &minode[i];
		if (mip->refCount > 0)
			iput(mip);
	}
	exit(0);
}