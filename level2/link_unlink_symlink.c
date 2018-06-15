int mylink()
{
	char oldFileNamecp[64], newFileNamecp[64];

	strcpy(oldFileNamecp, pathname);
	strcpy(newFileNamecp, newFileName);

	printf("oldFileNamecp=%s newFileNamecp=%s\n",
	       oldFileNamecp, newFileNamecp);

	int oldIno = getino(&dev, oldFileNamecp);

	if (oldIno == 0)
	{
		printf("file you want to link is not here.\n");
		return 0;
	}
	printf("1. getting the INODE of /a/b/c into memory: mip->minode[ ]\n");
	MINODE *oldmip = iget(dev, oldIno);

	printf("2. checking /a/b/c is a REG or LNK file (link to DIR is NOT allowed).\n");
	if (isDir(oldmip))
	{
		printf("this one is dir, NOT allowed\n");
		return 0;
	}
	printf("it is file. allowed\n");


	char child[64], parent[64], pathnamecp[64];
	char *testdir, *testchild;
	strcpy(pathnamecp, newFileNamecp);
	testdir = dirname(pathnamecp);
	strcpy(parent, testdir);

	strcpy(pathnamecp, newFileNamecp);
	testchild = basename(pathnamecp);
	strcpy(child, testchild);

	printf("[*make_dir]parent=%s child=%s\n", parent, child);
	printf("3. checking %s exists and is a DIR but %s does not yet exist in %s\n",
	       parent, child, parent);

	int newIno = getino(&dev, parent);

	if (newIno == 0)
	{
		printf("%s is not exists\n", parent);
		return 0;
	}

	MINODE *newpmip = iget(dev, newIno);

	if (!isDir(newpmip))
	{
		printf("%s is not DIR\n", parent);
		return 0;
	}

	newIno = getino(&dev, newFileNamecp);

	if (newIno != 0)
	{
		printf("%s is already exists in %s\n", child, parent);
		return 0;
	}

	my_link(newpmip, child, oldIno);
	oldmip->INODE.i_links_count ++;
	oldmip->dirty = 1;
	iput(oldmip);
	iput(newpmip);
}

int my_link (MINODE *pmip, char *myname, int oldIno)
{
	char buf[BLKSIZE];
	char namebuf[64];
	char *cp = NULL;
	printf("[my_link]pmip->INODE.i_block[0]=%d\n", pmip->INODE.i_block[0]);
	get_block(dev, pmip->INODE.i_block[0], buf);
	cp = buf;
	DIR *dp = (DIR *)buf;
	while ((cp + dp->rec_len) < (buf + BLKSIZE)) {
		printf("cp=%d buf=%d\n", cp, (buf + BLKSIZE));
		// print DIR entries to see what they are
		strncpy(namebuf, dp->name, dp->name_len);
		namebuf[dp->name_len] = 0;

		printf("%5d %7d %8d %4s\n",
		       dp->inode, dp->rec_len, dp->name_len, namebuf);

		cp += dp->rec_len;
		dp = (DIR *)cp;
	}

	strncpy(namebuf, dp->name, dp->name_len);
	namebuf[dp->name_len] = 0;
	printf("last dp: %5d %7d %8d %4s\n",
	       dp->inode, dp->rec_len, dp->name_len, namebuf);

	int need_length = 4 * ((8 + strlen(myname) + 3) / 4);

	int ideal_len = 4 * ((8 + dp->name_len + 3) / 4);

	int remain = dp->rec_len - ideal_len;

	printf("[my_link]need_length=%d ideal_len=%d remain=%d\n",
	       need_length, ideal_len, remain);

	printf("remain >= need_length\n");
	dp->rec_len = ideal_len;

	cp += dp->rec_len;
	dp = (DIR *)cp;

	dp->inode     = (u32)oldIno;
	dp->rec_len   = (u16)remain;
	dp->name_len  = (u8)strlen(myname);

	strcpy(dp->name, myname);

	printf("new entry: %5d %7d %8d %4s\n",
	       dp->inode, dp->rec_len, dp->name_len, dp->name);
	put_block(dev, pmip->INODE.i_block[0], buf);

	return 1;
}

int my_unlink()
{
	printf("(1). get pathname's INODE into memory\n");
	int ino = getino(&dev, pathname);
	MINODE *mip = iget(dev, ino);

	printf("(2). verify it's a FILE (REG or LNK), can not be a DIR;\n");
	if (S_ISDIR(mip->INODE.i_mode))
	{
		printf("[unlink] it is a dir. exit.\n");
		iput(mip);
		return 0;
	}

	printf("(3). decrement INODE's i_links_count by 1;\n");
	mip->INODE.i_links_count --;
	mip->dirty = 1;

	if (mip->INODE.i_links_count > 0)
	{
		printf("i_links_count>0. no need to rm\n");
	}
	else
	{
		printf("(4). if i_links_count == 0 ==> rm pathname\n");
		truncate(mip);
	}

	char pathnamecp[64], parent[64], child[64];
	char *temp;
	strcpy(pathnamecp, pathname);
	temp = dirname(pathnamecp);
	strcpy(parent, temp);

	strcpy(pathnamecp, pathname);
	temp = basename(pathnamecp);
	strcpy(child, temp);


	int pino = getino(&dev, parent);
	MINODE *pmip = iget(dev, pino);
	printf("Remove childName = basename(pathname) from the parent directory\n");
	printf("child=%s\n", child);

	rm_child(pmip, child);

	pmip->dirty = 1;
	iput(mip);
	iput(pmip);

}

int truncate(MINODE *mip) //mip
{
	printf("  1. release mip->INODE's data blocks\n");
	for (int i = 0; i < 12; i++)
	{
		int bno = mip->INODE.i_block[i];
		if (bno == 0) return 0;
		printf("[truncate]i=%d bno=%d\n", i, bno);

		mip->INODE.i_block[i] = 0;
		bdealloc(dev, bno); //bmap
	}

	int bno = mip->INODE.i_block[12];

	if (bno != 0)
	{
		printf("[truncate] i_block[12]=%d\n", bno);
		printf("[truncate] Indirect Blocks\n");
		int buf[256];
		get_block(dev, bno, buf);
		for (int i = 0; i < 256; i++)
		{
			if (buf[i] == 0) break;
			buf[i] = 0;
			bdealloc(dev, bno);
		}
	}

	bno = mip->INODE.i_block[13];

	if (bno != 0)
	{
		printf("[truncate] i_block[13]=%d\n", bno);
		printf("[truncate] double Indirect Blocks\n");
		int buf1[256], buf2[256];
		get_block(dev, bno, buf1);
		for (int i = 0; i < 256; i++)
		{
			if (buf1[i] == 0) break;
			get_block(fd, buf1[i], buf2);
			for (int j = 0; j < 256; j++)
			{
				if (buf2[j] == 0) break;
				printf("releasing %d\n", buf2[j]);
				buf2[i] = 0;
				bdealloc(dev, bno);
			}
			buf1[i] = 0;
		}
	}

	printf("2. update INODE's time field\n");
	mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);

	printf("3. set INODE's size to 0 and mark Minode[ ] dirty\n");
	mip->INODE.i_size = 0;
	mip->dirty = 1;
}

int my_symlink()
{
	char oldFileNamecp[64], newFileNamecp[64];

	strcpy(oldFileNamecp, pathname);
	strcpy(newFileNamecp, newFileName);

	printf("oldFileNamecp=%s newFileNamecp=%s\n",
	       oldFileNamecp, newFileNamecp);

	printf("(1). verify oldNAME exists (either a DIR or a REG file)");

	int oldIno = getino(&dev, oldFileNamecp);

	if (oldIno == 0)
	{
		printf("file you want to symlink is not here.\n");
		return 0;
	}

	char child[64], parent[64], pathnamecp[64];
	char *testdir, *testchild;
	strcpy(pathnamecp, newFileNamecp);
	testdir = dirname(pathnamecp);
	strcpy(parent, testdir);

	strcpy(pathnamecp, newFileNamecp);
	testchild = basename(pathnamecp);
	strcpy(child, testchild);

	printf("[symlink]parent=%s child=%s\n", parent, child);

	int newIno = getino(&dev, parent);

	if (newIno == 0)
	{
		printf("%s is not exists\n", parent);
		return 0;
	}

	MINODE *newpmip = iget(dev, newIno);

	if (!isDir(newpmip))
	{
		printf("%s is not DIR\n", parent);
		return 0;
	}

	newIno = getino(&dev, newFileNamecp);

	if (newIno != 0)
	{
		printf("%s is already exists in %s\n", child, parent);
		return 0;
	}
	printf("(2). creat a FILE\n");
	my_creat(newpmip, child);
	newpmip->dirty = 1;
	iput(newpmip);

	strcpy(oldFileNamecp, pathname);
	int childino = getino(&dev, newFileNamecp);
	MINODE *childmip = iget(dev, childino);

	printf("(3). change /x/y/z's type to LNK (0120000)=(1010.....)=0xA...\n");
	childmip->INODE.i_mode = 0xA1FF;
	childmip->INODE.i_size = strlen(oldFileNamecp);

	printf("(4). write the string oldNAME into the i_block[ ]\n");
	memcpy(childmip->INODE.i_block, oldFileNamecp, strlen(oldFileNamecp));

	printf("(5). write the INODE of /x/y/z back to disk.\n");
	childmip->dirty = 1;
	iput(childmip);

}