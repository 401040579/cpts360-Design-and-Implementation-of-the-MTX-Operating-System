int open_file()
{
	char pathnamecp[64], modestr[64];
	int mode;
	int rval;

	strcpy(pathnamecp, pathname);
	strcpy(modestr, newFileName);
	mode = atoi(modestr);
	printf("2. get pathname's inumber:\n");

	if (pathname[0] == '/') dev = root->dev; // root INODE's dev
	else dev = running->cwd->dev;

	int ino = getino(&dev, pathname);// NOTE: dev may change with mounting

	if (ino == 0)
	{
		printf("file is not here\n");
		return -1;
	}
	printf("3. get its Minode pointer\n");
	MINODE *mip = iget(dev, ino);

	printf("4. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.\n");

	if (!S_ISREG(mip->INODE.i_mode))
	{
		printf("%s is not REGULAR\n", pathname);
		iput(mip);
		return -1;
	}
	printf("permission checking.\n");
	INODE* ip = &mip->INODE;

	// User has 'all user' permissions
	// --- --- rwx
	int shift = 0;

	// User has 'owner' permissions
	if (running->uid == ip->i_uid)
	{
		// rwx --- ---
		shift = 6;
	}
	// User has 'group' permissions
	else if (running->gid == ip->i_gid)
	{
		// --- rwx ---
		shift = 3;
	}

	// 7 = 111
	u16 mask = mode & (7 << shift);

	int offset;
	printf("6. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:\n");
	switch (mode)
	{
	case 0:
		// 4 = 100
		if (((4 << shift) & mask) == 0 && running->uid != 0)
		{
			fprintf(stderr, "open: Permission denied\n");
			iput(mip);
			return -1;
		}

		offset = 0;
		break;

	case 1:
		// 2 = 010
		if (((2 << shift) & mask) == 0 && running->uid != 0)
		{
			fprintf(stderr, "open: Permission denied\n");
			iput(mip);
			return -1;
		}

		truncate(mip);
		offset = 0;
		break;

	case 2:
		// 6 = 110
		if (((6 << shift) & mask) == 0 && running->uid != 0)
		{
			fprintf(stderr, "open: Permission denied\n");
			iput(mip);
			return -1;
		}

		offset = 0;
		break;

	case 3:
		// 2 = 010
		if (((2 << shift) & mask) == 0 && running->uid != 0)
		{
			fprintf(stderr, "open: Permission denied\n");
			iput(mip);
			return -1;
		}

		offset = ip->i_size;
		break;

	default:
		fprintf(stderr, "open: failed to open '%s':"
		        " Unknown mode\n", pathname);
		iput(mip);
		return -1;
	}
	printf("permission OK.\n");

	OFT *oftp = NULL;
	/*
	printf("checking ino\n");
	for (int i = 0; i < NOFT ; i++)
	{
		oftp = &oft[i];
		if (oftp->refCount != 0)
			printf("i=%d oftpmptr->ino=%d\n", i, oftp->mptr->ino);
	}

	printf("ino checked\n");
	*/

	
	for (int i = 0; i < NOFT ; i++)
	{
		oftp = &oft[i];

		if (mode != 0
		        && oftp->refCount > 0
		        && oftp->mptr == mip
		        && oftp->mode != 0)
		{
			printf("4.1 Check whether the file is ALREADY opened with INCOMPATIBLE mode:\n");
			fprintf(stderr, "open: failed to open '%s':"
			        " File in use\n", pathname);
			iput(mip);
			return -1;
		}

		if (oftp->refCount == 0)
		{
			printf("5. allocate a FREE OpenFileTable (OFT) and fill in values:\n");
			oftp->mode = mode;
			oftp->offset = offset;
			oftp->refCount++;
			oftp->mptr = mip;
			break;
		}

		if (i == NOFT - 1)
		{
			fprintf(stderr, "open: failed to open '%s':"
			        " Too many files open\n", pathname);
			iput(mip);
			return -1;
		}
	}

	printf("7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL\n");

	int fd;
	for (int i = 0; fd < NFD; i++)
	{
		//printf("[open]i=%d", i);
		if (running->fd[i] == NULL)
		{
			fd = i;
			break;
		}

		if (i == NFD - 1)
		{
			fprintf(stderr, "open: failed to open '%s':"
			        " Process has reached file limit\n", pathname);
			iput(mip);
			return -1;
		}
	}

	printf("Add fd to process\n");
	running->fd[fd] = oftp;

	printf("8. update INODE's time field\n");
	if (mode == 0) ip->i_atime = time(0L);
	else ip->i_atime = ip->i_mtime = time(0L);

	printf("mark Minode[ ] dirty\n");
	mip->dirty = 1;
	//iput(mip);

	printf("9. return i=%d as the file descriptor\n", fd);
	return fd;
}

int my_close()
{
	char pathnamecp[64];
	int fd;
	strcpy(pathnamecp, pathname);
	fd = atoi(pathnamecp);

	return close_file(fd);
}

int close_file(int fd)
{
	printf("1. verify fd is within range.\n");
	if (fd < 0 || fd > NFD)
	{
		printf("fd is out of range.\n");
		return -1;
	}

	printf("2. verify running->fd[fd] is pointing at a OFT entry\n");
	OFT *oftp = NULL;
	for (int i = 0; i < NOFT; i++)
	{
		oftp = &oft[i];

		if (oftp->mptr == running->fd[fd]->mptr)
			break;

		if (i == NOFT - 1)
		{
			fprintf(stderr, "close: failed to close %d:"
			        " File not in oft\n", fd);
			return -1;
		}
	}

	oftp = running->fd[fd];
	running->fd[fd] = 0;
	oftp->refCount--;
	if (oftp->refCount > 0) return 0;

	// last user of this OFT entry ==> dispose of the Minode[]
	MINODE *mip = oftp->mptr;
	iput(mip);

	return 0;
}

int pfd()
{
	putchar('\n');
	printf("  fd   mode   offset   device   inode   refCount\n");
	printf("  --   ----   ------   ------   -----   --------\n");

	for (int i = 0; i < NOFT; i++)
	{
		OFT* oftp = &oft[i];

		if (oftp->refCount == 0)
			continue;

		printf("  %02d    ", i);

		switch (oftp->mode)
		{
		case 0:
			printf("RD");
			break;

		case 1:
			printf("WR");
			break;

		case 2:
			printf("RW");
			break;

		case 3:
			printf("AP");
			break;

		default:
			printf("??");
			break;
		}

		printf("    %06d     %02d     %05d  %02d\n",
		       oftp->offset, oftp->mptr->dev, oftp->mptr->ino, oftp->refCount);
	}
	putchar('\n');

	return 1;
}

int lseek_file()
{
	char fdstr[64], positionstr[64];
	int fd, position;

	strcpy(fdstr, pathname);
	strcpy(positionstr, newFileName);

	fd = atoi(fdstr);
	position = atoi(positionstr);


	printf("1. verify fd is within range.\n");
	if (fd < 0 || fd > NFD)
	{
		printf("fd is out of range.\n");
		return -1;
	}

	printf("2. verify running->fd[fd] is pointing at a OFT entry\n");
	OFT *oftp = NULL;
	for (int i = 0; i < NOFT; i++)
	{
		oftp = &oft[i];

		if (oftp->mptr == running->fd[fd]->mptr)
			break;

		if (i == NOFT - 1)
		{
			printf("File not in oft\n", fd);
			return -1;
		}
	}

	oftp = running->fd[fd];

	printf("change OFT entry's offset to position but make sure NOT to over run either end of the file.\n");

	int start = 0;
	int end = oftp->mptr->INODE.i_size;

	if (position < start)
		position = start;

	if (position > end)
		position = end;

	oftp->offset = position;

	printf("return originalPosition\n");
	return position;
}

//dup/dup2 fd gd share same oft