char * dirname1(char pathnamecp[], char dir[])
{

	int isRoot = 1;
	char parent[64], child[64], tparent[64], *namebuf;

	if (pathnamecp[0] == '/') strcpy(parent, "/");
	else strcpy(parent, "");
	namebuf = strtok(pathnamecp, "/");
	strcat(parent, namebuf);
	strcpy(child, namebuf);
	while (namebuf = strtok(NULL, "/"))
	{
		isRoot = 0;
		strcpy(child, namebuf);
		strcat(parent, "/");
		strcat(parent, child);
		//printf("[make_dir]pause0\n");		getchar();

	}
	//printf("[*make_dir]parent=%s child=%s\n", parent, child);
	//printf("[make_dir]pause1\n");		getchar();

	if (strcmp(parent, child) != 0 && isRoot == 0)
	{
		strncpy(tparent, parent, strlen(parent) - strlen(child) - 1);
		tparent[strlen(parent) - strlen(child) - 1] = 0;
		strcpy(parent, tparent);
	}
	else if (strcmp(parent, child) == 0)
	{
		strcpy(parent, "");
	}
	else
	{
		strcpy(parent, "/");
	}
	printf("[*make_dir]parent=%s child=%s\n", parent, child);
	strcpy(pathnamecp, parent);
	printf("pathnamecp=%s\n", pathnamecp);
	strcpy(dir, parent);
	return pathnamecp;
}

char * basename2(char pathnamecp[], char base[])
{

	int isRoot = 1;
	char parent[64], child[64], tparent[64], *namebuf;

	if (pathnamecp[0] == '/') strcpy(parent, "/");
	else strcpy(parent, "");
	namebuf = strtok(pathnamecp, "/");
	strcat(parent, namebuf);
	strcpy(child, namebuf);

	while (namebuf = strtok(NULL, "/"))
	{
		isRoot = 0;
		strcpy(child, namebuf);
		strcat(parent, "/");
		strcat(parent, child);
		//printf("[make_dir]pause0\n");		getchar();

	}
	//printf("[*make_dir]parent=%s child=%s\n", parent, child);
	//printf("[make_dir]pause1\n");		getchar();

	if (strcmp(parent, child) != 0 && isRoot == 0)
	{
		strncpy(tparent, parent, strlen(parent) - strlen(child) - 1);
		tparent[strlen(parent) - strlen(child) - 1] = 0;
		strcpy(parent, tparent);
	}
	else if (strcmp(parent, child) == 0)
	{
		strcpy(parent, "");
	}
	else
	{
		strcpy(parent, "/");
	}
	printf("[*make_dir]parent=%s child=%s\n", parent, child);
	strcpy(pathnamecp, child);
	printf("pathnamecp=%s\n", pathnamecp);
	strcpy(base, child);
	return pathnamecp;
}

int make_dir()
{
	//MINODE *mip;
	//int isRoot = 1;
	char pathnamecp[64], parent[64], child[64], tparent[64], *namebuf;



	//strcpy(pathnamecp, pathname);
	//printf("[**make_dir]pathname=%s pathnamecp=%s\n", pathname, pathnamecp);


	/*
	1. pahtname = "/a/b/c" start mip = root;         dev = root->dev;
	            =  "a/b/c" start mip = running->cwd; dev = running->cwd->dev;
	*/
	if (pathname[0] == '/')
	{
		//mip = iget(dev, 2);
		dev = root->dev;
	}
	else
	{
		//mip = iget(running->cwd->dev, running->cwd->ino);
		dev = running->cwd->dev;
	}
	//iput(mip);

	/*
	2. Let
	     parent = dirname(pathname);   parent= "/a/b" OR "a/b"
	     child  = basename(pathname);  child = "c"
	*/
	/*
		if (pathnamecp[0] == '/') strcpy(parent, "/");
		else strcpy(parent, "");
		namebuf = strtok(pathnamecp, "/");
		strcat(parent, namebuf);
		strcpy(child, namebuf);
		while (namebuf = strtok(NULL, "/"))
		{
			isRoot = 0;
			strcpy(child, namebuf);
			strcat(parent, "/");
			strcat(parent, child);
			//printf("[make_dir]pause0\n");		getchar();

		}
		//printf("[*make_dir]parent=%s child=%s\n", parent, child);
		//printf("[make_dir]pause1\n");		getchar();

		if (strcmp(parent, child) != 0 && isRoot == 0)
		{
			strncpy(tparent, parent, strlen(parent) - strlen(child) - 1);
			tparent[strlen(parent) - strlen(child) - 1] = 0;
			strcpy(parent, tparent);
		}
		else if (strcmp(parent, child) == 0)
		{
			strcpy(parent, "");
		}
		else
		{
			strcpy(parent, "/");
		}
	*/

	char *testdir, *testchild;
	strcpy(pathnamecp, pathname);
	testdir = dirname(pathnamecp);
	strcpy(parent, testdir);

	strcpy(pathnamecp, pathname);
	testchild = basename(pathnamecp);
	strcpy(child, testchild);


	printf("[*make_dir]parent=%s child=%s\n", parent, child);
	//getchar();


	//printf("[make_dir]pause2\n");		getchar();
	/*
	3. Get the In_MEMORY minode of parent:

	         pino  = getino(&dev, parent);
	         pip   = iget(dev, pino);

	   Verify : (1). parent INODE is a DIR (HOW?)   AND
	            (2). child does NOT exists in the parent directory (HOW?);
	*/
	// 1. parent must exist.
	int pino = getino(&dev, parent);
	if (pino == 0)
	{
		printf("[make_dir]parent not exist. pino=%d\n", pino);
		return 0;
	}

	// 2. parent must be dir
	MINODE *pmip = iget(dev, pino);

	if (!S_ISDIR(pmip->INODE.i_mode))
	{
		printf("[make_dir]pip is not a dir\n");
		iput(pmip);
		return 0;
	}
	printf("[make_dir]pip is a dir\n");

	// 3. child must not inside the parent
	//getchar();
	int cino = search(pmip, child);
	//putchar('1');getchar();
	if (cino != 0)
	{
		printf("[make_dir]child is already in dir\n");
		iput(pmip);
		return 0;
	}

	if (strcmp(child, "/") == 0)
	{
		printf("[make_dir] can't make / sign for dir\n");
		iput(pmip);
		return 0;
	}

	// 4. call mymkdir(pip, child);
	mymkdir(pmip, child);

	/*
	5. inc parent inodes's link count by 1;
	   touch its atime and mark it DIRTY
	*/
	INODE *pip = &pmip->INODE;
	// Child's '..' links to parent, so increment link count
	pip->i_links_count++;

	// Set parent's last time of access to current time
	pip->i_atime = time(0L);

	// Parent memory-inode now has child in its directory
	// but the disk-inode does not, hence parent is dirty.
	pmip->dirty = 1;

	// 6. iput(pip);
	iput(pmip);
}

int mymkdir(MINODE *pmip, char *name)
{
	//1. pip points at the parent minode[] of "/a/b", name is a string "c")

	/*
	2. allocate an inode and a disk block for the new directory;
	    ino = ialloc(dev);
	    bno = balloc(dev);
	*/
	int ino = ialloc(dev);
	int bno = balloc(dev);

	/*
	3. mip = iget(dev, ino) to load the inode into a minode[] (in order to
	wirte contents to the INODE in memory).
	*/
	printf("[mymkdir]ino=%d bno=%d\n", ino, bno);
	MINODE *mip = iget(dev, ino); //new dir's mip

	//4. Write contents to mip->INODE to make it as a DIR.
	INODE *ip = &mip->INODE;

	ip->i_mode = 0x41ED;		// OR 040755: DIR type and permissions
	ip->i_uid  = running->uid;	// Owner uid
	ip->i_gid  = running->gid;	// Group Id
	ip->i_size = BLKSIZE;		// Size in bytes
	ip->i_links_count = 2;	    // Links count=2 because of . and ..
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
	ip->i_blocks = 2;           // LINUX: Blocks count in 512-byte chunks
	ip->i_block[0] = bno;       // new DIR has one data block
	for (int i = 1; i < 15 ; i++)
	{
		ip->i_block[i] = 0; // i_block[1] to i_block[14] = 0;
	}

	//5. iput(mip); which should write the new INODE out to disk.
	mip->dirty = 1;               // mark minode dirty
	iput(mip);                    // write INODE to disk


	/***** create data block for new DIR containing . and .. entries ******

	6. Write . and .. entries into a buf[ ] of BLKSIZE

	   | entry .     | entry ..                                             |
	   ----------------------------------------------------------------------
	   |ino|12|1|.   |pino|1012|2|..                                        |
	   ----------------------------------------------------------------------

	   Then, write buf[ ] to the disk block bno;
	   */
	char buf[BLKSIZE];
	char *cp;
	printf("[mymkdir]making data block\n");
	get_block(dev, bno, buf);
	cp = buf;
	DIR *dp = (DIR *)buf;

	dp->inode     = (u32)ino;
	dp->rec_len   = (u16)12;
	dp->name_len  = (u8)1;
	dp->name[0]   = '.';
	printf(" . entry: %5d %7d %8d %4s\n",
	       dp->inode, dp->rec_len, dp->name_len, dp->name);
	cp += dp->rec_len;       // advance cp by rec_len BYTEs
	dp = (DIR*)cp;           // pull dp along to the next record

	dp->inode     = (u32)pmip->ino;
	dp->rec_len   = (u16)1012; // 1024-12

	dp->name_len  = (u8)2;
	dp->name[0]   = '.';
	dp->name[1]   = '.';
	printf(" .. entry: %5d %7d %8d %4s\n",
	       dp->inode, dp->rec_len, dp->name_len, dp->name);
	printf("writing data block %d to disk\n", bno);
	put_block(dev, bno, buf);

	/*
	7. Finally, enter name ENTRY into parent's directory by
	            enter_name(pip, ino, name);
	*/
	printf("mip->INODE.i_block[0]=%d\n", mip->INODE.i_block[0]);
	printf("Enter: parent = (dev=%d ino=%d) name=%s\n", pmip->dev, pmip->ino,
	       name);
	enter_name(pmip, ino, name); //pmip, child_ino, child_name
}

int enter_name(MINODE *pmip, int myino, char *myname) //parent mip,cino,cname
{
	char buf[BLKSIZE];
	INODE *pip = &pmip->INODE;
	int bno;
	int i;
	char *cp;
	/*
	8. int enter_name(MINODE *pip, int myino, char *myname)
	{
	For each data block of parent DIR do { // assume: only 12 direct blocks

	 if (i_block[i]==0) BREAK;
	*/
	for (i = 0; i < 12; i++)
	{
		bno = pip->i_block[i];
		printf("[enter_name]bno=%d\n", bno);

		if (bno == 0) break;

		//(1). get parent's data block into a buf[];
		get_block(pmip->dev, bno, buf);

		/*
		(2). EXT2 DIR entries: Each DIR entry has rec_len and name_len. Each entry's
		     ideal length is
		                     IDEAL_LEN = 4*[ (8 + name_len + 3)/4 ].
		     All DIR entries in a data block have rec_len = IDEAL_LEN, except the last
		     entry. The rec_len of the LAST entry is to the end of the block, which may
		     be larger than its IDEAL_LEN.

		  --|-4---2----2--|----|---------|--------- rlen ->------------------------|
		    |ino rlen nlen NAME|.........|ino rlen nlen|NAME                       |
		  --------------------------------------------------------------------------
		*/

		DIR *dp = (DIR *)buf;
		cp = buf;
		printf("cp=%d buf=%d\n", cp, buf);
		char namebuf[256];
		// step to LAST entry in block: int blk = parent->INODE.i_block[i];
		printf("step to LAST entry in data block %d\n", bno);

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
		// dp NOW points at last entry in block

		int need_length = 4 * ((8 + strlen(myname) + 3) / 4);

		// or called last_len
		int ideal_len = 4 * ((8 + strlen(namebuf) + 3) / 4);

		int remain = dp->rec_len - ideal_len;

		printf("[enter_name]need_length=%d ideal_len=%d remain=%d\n",
		       need_length, ideal_len, remain);

		if (remain >= need_length)
		{
			printf("remain >= need_length\n");
			dp->rec_len = ideal_len;

			cp += dp->rec_len;
			dp = (DIR *)cp;

			dp->inode     = (u32)myino; //cino
			//dp->rec_len   = (u16)(BLKSIZE - (cp - buf));

			dp->rec_len   = (u16)remain;
			dp->name_len  = (u8)strlen(myname);

			strcpy(dp->name, myname);

			printf("new entry: %5d %7d %8d %4s\n",
			       dp->inode, dp->rec_len, dp->name_len, dp->name);
			put_block(dev, bno, buf);
			return 1;
		}
	}



	//(5). Reach here means: NO space in existing data block(s)
	/*
	Allocate a new data block; INC parent's isze by 1024;
	Enter new entry as the first entry in the new data block with rec_len=BLKSIZE.

	|-------------------- rlen = BLKSIZE -------------------------------------
	|myino rlen nlen myname                                                  |
	--------------------------------------------------------------------------
	*/
	bno = balloc(dev);
	pip->i_block[i] = bno;
	pip->i_size += BLKSIZE;
	pmip->dirty = 1; // needed?

	// Get parent's data block into a buf[]
	get_block(dev, bno, buf);
	cp = buf;
	dp = (DIR*)buf;

	dp->inode = (u32)myino;
	dp->rec_len = (u16)BLKSIZE;
	dp->name_len = (u8)strlen(myname);
	strcpy(dp->name, myname);

	// Write data block to disk
	put_block(dev, bno, buf);
	return 1;
	//printf("pause\n");		putchar();
	//(6).Write data block to disk;

}

int creat_file()
{
	MINODE *mip;
	//int isRoot = 1;
	char pathnamecp[64], parent[64], child[64], tparent[64], *namebuf;
	//strcpy(pathnamecp, pathname);
	//printf("[**creat_file]pathname=%s pathnamecp=%s\n", pathname, pathnamecp);


	/*
	1. pahtname = "/a/b/c" start mip = root;         dev = root->dev;
	            =  "a/b/c" start mip = running->cwd; dev = running->cwd->dev;
	*/
	if (pathname[0] == '/')
	{
		mip = iget(dev, 2);
		dev = root->dev;
	}
	else
	{
		mip = iget(running->cwd->dev, running->cwd->ino);
		dev = running->cwd->dev;
	}
	//iput(mip);
	/*
	2. Let
	     parent = dirname(pathname);   parent= "/a/b" OR "a/b"
	     child  = basename(pathname);  child = "c"
	*/

	/*
		if (pathnamecp[0] == '/') strcpy(parent, "/");
		else strcpy(parent, "");
		namebuf = strtok(pathnamecp, "/");
		strcat(parent, namebuf);
		strcpy(child, namebuf);
		while (namebuf = strtok(NULL, "/"))
		{
			isRoot = 0;
			strcpy(child, namebuf);
			strcat(parent, "/");
			strcat(parent, child);
			//printf("[make_dir]pause0\n");		getchar();

		}
		//printf("[*make_dir]parent=%s child=%s\n", parent, child);
		//printf("[make_dir]pause1\n");		getchar();

		if (strcmp(parent, child) != 0 && isRoot == 0)
		{
			strncpy(tparent, parent, strlen(parent) - strlen(child) - 1);
			tparent[strlen(parent) - strlen(child) - 1] = 0;
			strcpy(parent, tparent);
		}
		else if (strcmp(parent, child) == 0)
		{
			strcpy(parent, "");
		}
		else
		{
			strcpy(parent, "/");
		}
		printf("[*creat_file]parent=%s child=%s\n", parent, child);
		//printf("[make_dir]pause2\n");		getchar();
		*/

	char *testdir, *testchild;
	strcpy(pathnamecp, pathname);
	testdir = dirname(pathnamecp);
	strcpy(parent, testdir);

	strcpy(pathnamecp, pathname);
	testchild = basename(pathnamecp);
	strcpy(child, testchild);


	printf("[*make_dir]parent=%s child=%s\n", parent, child);
	/*
	3. Get the In_MEMORY minode of parent:

	         pino  = getino(&dev, parent);
	         pip   = iget(dev, pino);

	   Verify : (1). parent INODE is a DIR (HOW?)   AND
	            (2). child does NOT exists in the parent directory (HOW?);
	*/
	// 1. parent must exist.
	int pino = getino(&dev, parent);
	if (pino == 0)
	{
		printf("[creat_file]pino=%d\n", pino);
		return 0;
	}

	// 2. parent must be dir
	MINODE *pmip = iget(dev, pino);

	if (!isDir(pmip))
	{
		printf("[creat_file]pip is not a dir\n");
		return 0;
	}
	printf("[creat_file]pip is a dir\n");

	// 3. child must not inside the parent
	//getchar();
	int cino = search(pmip, child);
	//putchar('1');getchar();
	if (cino != 0)
	{
		printf("[creat_file]child is already in dir\n");
		return 0;
	}

	// 4. call mymkdir(pip, child);
	my_creat(pmip, child);

	/*
	5. inc parent inodes's link count by 1;
	   touch its atime and mark it DIRTY
	*/
	INODE *pip = &pmip->INODE;

	// Child's '..' links to parent, so increment link count

	//Do NOT increment parent's links_count
	//pip->i_links_count++;

	// Set parent's last time of access to current time
	pip->i_atime = time(0L);

	// Parent memory-inode now has child in its directory
	// but the disk-inode does not, hence parent is dirty.
	pmip->dirty = 1;

	// 6. iput(pip);
	iput(pmip);
}

int my_creat (MINODE *pmip, char *myname)
{
	//1. pip points at the parent minode[] of "/a/b", name is a string "c")

	/*
	2. allocate an inode for the new file;
	    ino = ialloc(dev);
	*/
	int ino = ialloc(dev); // new file's inode
	//int bno = balloc(dev);


	/*
	3. mip = iget(dev, ino) to load the inode into a minode[] (in order to
	wirte contents to the INODE in memory).
	*/
	printf("[my_creat]ino=%d\n", ino);
	MINODE *mip = iget(dev, ino); //new file's mip

	//4. Write contents to mip->INODE to make it as a DIR.
	INODE *ip = &mip->INODE;

	ip->i_mode = 0x81A4;		// INODE's file type = 0x81A4 OR 0100644
	ip->i_uid  = running->uid;	// Owner uid
	ip->i_gid  = running->gid;	// Group Id
	ip->i_size = 0;		// Size in bytes
	ip->i_links_count = 1;	    // Links count=2 because of . and ..
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
	ip->i_blocks = 0;           // LINUX: Blocks count in 512-byte chunks
	ip->i_block[0] = 0;       // file no block
	for (int i = 1; i < 15 ; i++)
	{
		ip->i_block[i] = 0; // i_block[1] to i_block[14] = 0;
	}

	//5. iput(mip); which should write the new INODE out to disk.
	mip->dirty = 1;               // mark minode dirty
	iput(mip);                    // write INODE to disk


	//creat child file in parent dir

	char buf[BLKSIZE];
	char namebuf[64];
	char *cp = NULL;
	printf("[my_creat]pmip->INODE.i_block[0]=%d\n", pmip->INODE.i_block[0]);
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
	// or called last_len
	int ideal_len = 4 * ((8 + dp->name_len + 3) / 4);

	int remain = dp->rec_len - ideal_len;

	printf("[enter_name]need_length=%d ideal_len=%d remain=%d\n",
	       need_length, ideal_len, remain);

	printf("remain >= need_length\n");
	dp->rec_len = ideal_len;

	cp += dp->rec_len;
	dp = (DIR *)cp;

	dp->inode     = (u32)ino; //cino
	//dp->rec_len   = (u16)(BLKSIZE - (cp - buf));

	dp->rec_len   = (u16)remain;
	dp->name_len  = (u8)strlen(myname);

	strcpy(dp->name, myname);

	printf("new entry: %5d %7d %8d %4s\n",
	       dp->inode, dp->rec_len, dp->name_len, dp->name);
	put_block(dev, pmip->INODE.i_block[0], buf);
	return 1;
}