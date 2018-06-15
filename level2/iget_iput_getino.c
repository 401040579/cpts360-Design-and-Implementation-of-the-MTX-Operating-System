int get_block(int fd, int blk, char buf[ ])
{
	lseek(fd, (long)blk * BLKSIZE, 0);
	read(fd, buf, BLKSIZE);
}

int put_block(int fd, int blk, char buf[ ])
{
	lseek(fd, (long)blk * BLKSIZE, 0);
	write(fd, buf, BLKSIZE);
}

int tst_bit(char *buf, int bit)
{
	int i, j;
	i = bit / 8; j = bit % 8;
	if (buf[i] & (1 << j))
		return 1;
	return 0;
}

int set_bit(char *buf, int bit)
{
	int i, j;
	i = bit / 8; j = bit % 8;
	buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
	int i, j;
	i = bit / 8; j = bit % 8;
	buf[i] &= ~(1 << j);
}

// search in minode, if found return mip, else allocat a new minode
MINODE *iget(int dev, int ino)
{

	int i, blk, disp;
	char buf[BLKSIZE];
	MINODE *mip;
	INODE *ip;

	for (i = 0; i < NMINODE; i++) {
		mip = &minode[i];
		if (mip->dev == dev && mip->ino == ino && mip->refCount) {
			mip->refCount++;
			printf("found [dev=%d ino=%d] as minode[%d] in core\n", dev, ino, i);
			return mip;
		}
	}

	for (i = 0; i < NMINODE; i++) {
		mip = &minode[i];
		if (mip->refCount == 0) {
			printf("allocating NEW minode[%d] for [dev=%d ino=%d]\n", i, dev, ino);
			mip->refCount = 1;
			mip->dev = dev;
			mip->ino = ino;  // assing to (dev, ino)
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

// dispose of a minode[] pointed by mip
int iput(MINODE *mip)
{
	//put mem-inode back

	int blk, disp;
	char buf[BLKSIZE];
	INODE *ip;

	mip->refCount--;

	if (mip->refCount > 0) return;
	if (!mip->dirty)       return;

	/*(3).   write INODE back to disk */

	printf("[**iput]iput: dev=%d ino=%d\n", mip->dev, mip->ino);

	blk  = (mip->ino - 1) / 8 + iblock;
	disp = (mip->ino - 1) % 8;

	get_block(mip->dev, blk, buf);
	ip = (INODE *)buf + disp;
	*ip = mip->INODE;
	put_block(mip->dev, blk, buf);
}

int search(MINODE *mip, char *name) //(mip, name[i])
{
	char lbuf[BLKSIZE];
	char namebuf[256];
	int mblock;
	char *cp;
	//add for loop can do i_block[0-15]
	printf("[**search]:search for %s in MINODE = [dev=%d, ino=%d]\n", name, mip->dev, mip->ino);

	mblock = mip->INODE.i_block[0];
	printf("[**search]:mblock = %d\n", mblock);

	if (mblock == 0) return 0;

	get_block(mip->dev, mblock, lbuf);
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

		if (strcmp(name, namebuf) == 0)
			return dp->inode;

		cp += dp->rec_len;
		dp = (DIR*) cp;
		//printf("pause[search]\n");getchar();
	}
	return 0;
}

int tokenize(char *pathname)
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
	printf("[**tokenize]n=%d " , n);
	for (int j = 0; j < n; j++)
	{
		printf("%4s ", name[j]);
	}
	printf("\n");
}

int getino(int *dev, char *pathname)
{
	int i, ino, blk, disp;
	char buf[BLKSIZE];
	INODE *ip;
	MINODE *mip;

	printf("getino: pathname=%s\n", pathname);
	if (strcmp(pathname, "/") == 0)
		return 2;

	if (pathname[0] == '/')
		mip = iget(*dev, 2);
	else
	{
		mip = iget(running->cwd->dev, running->cwd->ino);
		//ino = running->cwd->ino;
	}

	strcpy(buf, pathname);
	tokenize(buf); // n = number of token strings

	for (i = 0; i < n; i++) {
		printf("===========================================\n");
		printf("getino: i=%d name[%d]=%s\n", i, i, name[i]); //used to be kcwname

		ino = search(mip, name[i]);

		if (ino == 0) {
			iput(mip);
			printf("name %s does not exist\n", name[i]);
			return 0;
		}
		iput(mip);
		mip = iget(*dev, ino);

	}
	//printf("[***]ino[getino]=%d\n", ino);
	iput(mip);
	return ino;
}

/********** Functions as BEFORE ***********/
/*
int get_block(int fd, int blk, char buf[ ])
{
	lseek(fd, (long)blk * BLKSIZE, 0);
	read(fd, buf, BLKSIZE);
}

int put_block(int fd, int blk, char buf[ ])
{
	lseek(fd, (long)blk * BLKSIZE, 0);
	write(fd, buf, BLKSIZE);
}

int tst_bit(char *buf, int bit)
{
	int i, j;
	i = bit / 8; j = bit % 8;
	if (buf[i] & (1 << j))
		return 1;
	return 0;
}

int set_bit(char *buf, int bit)
{
	int i, j;
	i = bit / 8; j = bit % 8;
	buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
	int i, j;
	i = bit / 8; j = bit % 8;
	buf[i] &= ~(1 << j);
}

MINODE *iget(int dev, int ino)
{
	int i, blk, disp;
	char buf[BLKSIZE];
	MINODE *mip;
	INODE *ip;
	for (i = 0; i < NMINODE; i++) {
		mip = &minode[i];
		if (mip->dev == dev && mip->ino == ino) {
			mip->refCount++;
			printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
			return mip;
		}
	}
	for (i = 0; i < NMINODE; i++) {
		mip = &minode[i];
		if (mip->refCount == 0) {
			printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
			mip->refCount = 1;
			mip->dev = dev; mip->ino = ino;  // assing to (dev, ino)
			mip->dirty = mip->mounted = mip->mountPtr = 0; //should be mptr?
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

int iput(MINODE *mip)  // dispose of a minode[] pointed by mip
{
	int i, blk, disp;
	char buf[BLKSIZE];

	mip->refCount--;
	if (mip->refCount > 0) return;
	if (!mip->dirty)       return;

	printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino);

	blk  = (mip->ino - 1) / 8 + iblock;
	disp = (mip->ino - 1) % 8;
	get_block(mip->dev, blk, buf);

	ip = (INODE *)buf + disp;
	*ip = mip->INODE;

	put_block(mip->dev, blk, buf);
}

int search(MINODE *mip, char *name)
{

}

int getino(int *dev, char *pathname)
{
	int i, ino, blk, disp;
	char buf[BLKSIZE];
	INODE *ip;
	MINODE *mip;

	printf("getino: pathname=%s\n", pathname);
	if (strcmp(pathname, "/") == 0)
		return 2;

	if (pathname[0] == '/')
		mip = iget(*dev, 2);
	else
		mip = iget(running->cwd->dev, running->cwd->ino);

	strcpy(buf, pathname);
	tokenize(buf); // n = number of token strings

	for (i = 0; i < n; i++) {
		printf("===========================================\n");
		printf("getino: i=%d name[%d]=%s\n", i, i, kcwname[i]);

		ino = search(mip, name[i]);

		if (ino == 0) {
			iput(mip);
			printf("name %s does not exist\n", name[i]);
			return 0;
		}
		iput(mip);
		mip = iget(*dev, ino);
	}
	return ino;
}
*/