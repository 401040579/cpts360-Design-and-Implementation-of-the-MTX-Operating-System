int write_file()
{
	char fdstr[64], stringstr[64];
	int fd, nbytes;
	char buf[BLKSIZE];

	strcpy(fdstr, pathname);
	strcpy(stringstr, newFileName);
	strcpy(buf, stringstr);
	fd = atoi(fdstr);

	printf("fd=%d string=%s\n", fd, stringstr);

	OFT *oftp = running->fd[fd];

	if (oftp->mode == 1)
	{

	}
	else if (oftp->mode == 2)
	{

	}
	else if (oftp->mode == 3)
	{

	}
	else
	{
		printf("fd is not WR or RW or APPEND mode\n");
		return -1;
	}

	return (mywrite(fd, buf, strlen(buf)));
}

int mywrite(int fd, char buf[ ], int nbytes)
{

	char wbuf[BLKSIZE];
	int orbytes = nbytes;
	int count = 0;
	MINODE *mip = running->fd[fd]->mptr;
	int offset = running->fd[fd]->offset;
	char *cq = buf;                // cq points at buf[ ]
	int lbk, blk, startByte, remain;
	OFT *oftp = running->fd[fd];

	printf("nbytes=%d\n", nbytes);

	while (nbytes > 0 ) {

		//compute LOGICAL BLOCK (lbk) and the startByte in that lbk:

		lbk       = oftp->offset / BLKSIZE;
		startByte = oftp->offset % BLKSIZE;

		printf("lbk=%d startByte=%d\n", lbk, startByte);

		// I only show how to write DIRECT data blocks, you figure out how to
		// write indirect and double-indirect blocks.

		if (lbk < 12) {                        // direct block
			if (mip->INODE.i_block[lbk] == 0) {  // if no data block yet
				mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a block

				// write a block of 0's to blk on disk: OPTIONAL for data block
				//                                      but MUST for I or D blocks
			}
			blk = mip->INODE.i_block[lbk];      // blk should be a disk block now
			printf("blk=%d\n", blk);
		}
		else if (lbk >= 12 && lbk < 256 + 12) {
			// indirect blocks
			int addressbuf[256];

			if (mip->INODE.i_block[12] == 0)
			{
				mip->INODE.i_block[12] = balloc(mip->dev);
			}

			get_block(dev, mip->INODE.i_block[12], addressbuf);

			if (addressbuf[lbk - 12] == 0)
			{
				addressbuf[lbk - 12] = balloc(mip->dev);
			}

			blk = addressbuf[lbk - 12];

			put_block(dev, mip->INODE.i_block[12], addressbuf);

			printf("blk=%d\n", blk);
		}
		else {
			// double indirect blocks */
			int addressbuf1[256];
			int addressbuf2[256];

			if (mip->INODE.i_block[13] == 0)
			{
				mip->INODE.i_block[13] = balloc(mip->dev);
			}

			get_block(dev, mip->INODE.i_block[13], addressbuf1);

			if (addressbuf1[(lbk - (256 + 12)) / 256] == 0)
			{
				addressbuf1[(lbk - (256 + 12)) / 256] = balloc(mip->dev);
			}

			put_block(dev, mip->INODE.i_block[13], addressbuf1);

			get_block(dev, addressbuf1[(lbk - (256 + 12)) / 256], addressbuf2);

			if (addressbuf2[(lbk - (256 + 12)) % 256] == 0)
			{
				addressbuf2[(lbk - (256 + 12)) % 256] = balloc(mip->dev);
			}

			blk = addressbuf2[(lbk - (256 + 12)) % 256];

			put_block(dev, addressbuf1[(lbk - (256 + 12)) / 256], addressbuf2);

			printf("blk=%d\n", blk);
		}

		/* all cases come to here : write to the data block */
		get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]
		char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
		remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

		/*
				while (remain > 0) {              // write as much as remain allows
					*cp++ = *cq++;              // cq points at buf[ ]
					nbytes--; remain--;         // dec counts
					//printf("nbytes=%d offset=%d mip->INODE.i_size=%d\n", nbytes, offset, mip->INODE.i_size);
					oftp->offset++;             // advance offset

					if (oftp->offset > mip->INODE.i_size)  // especially for RW|APPEND mode
						mip->INODE.i_size++;    // inc file size (if offset > fileSize)
					if (nbytes <= 0) break;     // if already nbytes, break
				}
		*/
		if (remain > 0)
		{
			if (nbytes >= remain)
			{
				memcpy(cp, cq, remain);
				nbytes -= remain;
				oftp->offset += remain;
				remain = 0;

				if (oftp->offset > mip->INODE.i_size)
					mip->INODE.i_size = oftp->offset;

			}
			else //nbytes < remain
			{
				memcpy(cp, cq, nbytes);
				remain -= nbytes;
				oftp->offset += nbytes;
				nbytes = 0;

				if (oftp->offset > mip->INODE.i_size)
					mip->INODE.i_size = oftp->offset;

			}
		}

		put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk

		// loop back to while to write more .... until nbytes are written
	}

	mip->dirty = 1;       // mark mip dirty for iput()
	printf("wrote %d char into file descriptor fd=%d\n", orbytes, fd);
	return orbytes;
}

int mycp()
{
	char src[64], dest[64];
	char buf[1024];

	strcpy(src, pathname);
	strcpy(dest, newFileName);

	printf("1. fd = open src for READ;\n");
	strcpy(pathname, src);
	strcpy(newFileName, "0");
	int fd = open_file();

	if (fd < 0) return -1;

	printf("2. gd = open dst for WR|CREAT; \n");
	printf("2.1 you may have to creat the dst file first\n");
	strcpy(pathname, dest);
	creat_file();

	printf("2.2 then open it for WR\n");
	strcpy(pathname, dest);
	strcpy(newFileName, "2");
	int gd = open_file();

	printf("[mycp]fd=%d gd=%d\n", fd, gd);
	while (n = myread(fd, buf, BLKSIZE))
	{
		mywrite(gd, buf, n);
	}

	close_file(fd);
	close_file(gd);
}

int mymv()
{
	/*
	1. verify src exists; get its INODE in ==> you already know its dev
	2. check whether src is on the same dev as src

	          CASE 1: same dev:
	3. Hard link dst with src (i.e. same INODE number)
	4. unlink src (i.e. rm src name from its parent directory and reduce INODE's
	           link count by 1).

	          CASE 2: not the same dev:
	3. cp src to dst
	4. unlink src
	*/
	char src[64], dest[64];

	strcpy(src, pathname);
	strcpy(dest, newFileName);

	int ino = getino(&dev, src);
	if (ino == 0) return -1;

	MINODE *mip = iget(dev, ino);

	if (mip->dev == running->cwd->dev)
	{
		printf("CASE 1: same dev:\n");
		strcpy(pathname, src);
		strcpy(newFileName, dest);
		mylink();

		strcpy(pathname, src);
		my_unlink();
	}
	else
	{
		printf("CASE 2: not the same dev:\n");
		strcpy(pathname, src);
		strcpy(newFileName, dest);
		mycp();

		strcpy(pathname, src);
		my_unlink();
	}

	iput(mip);
	return 0;
}