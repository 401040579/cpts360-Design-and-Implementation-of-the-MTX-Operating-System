int read_file()
{
	char fdstr[64], positionstr[64];
	int fd, position, nbytes;
	char buf[BLKSIZE];

	strcpy(fdstr, pathname);
	strcpy(positionstr, newFileName);

	fd = atoi(fdstr);
	position = atoi(positionstr);

	printf("fd=%d position=%d\n", fd, position);

	OFT *oftp = running->fd[fd];

	if (oftp->mode == 0)
	{

	}
	else if (oftp->mode == 2)
	{

	}
	else
	{
		printf("fd is not RD or RW\n");
		return -1;
	}
	nbytes = position;
	int count = myread(fd, buf, nbytes);
	buf[count]=0;
	printf("%s\n", buf);
	return count;
}


int myread(int fd, char *buf, int nbytes)
{
	char readbuf[BLKSIZE];
	int avil;
	int count = 0;
	MINODE *mip = running->fd[fd]->mptr;
	int fileSize = mip->INODE.i_size;
	printf("[myread]fileSize=%d\n", fileSize);
	int offset = running->fd[fd]->offset;
	avil = fileSize - offset; // number of bytes still available in file.
	char *cq = buf;                // cq points at buf[ ]
	int lbk, blk, startByte, remain;

	OFT *oftp = running->fd[fd];
	printf("nbytes=%d avil=%d\n", nbytes, avil);



	while (nbytes && avil) {

		//Compute LOGICAL BLOCK number lbk and startByte in that block from offset;

		lbk       = oftp->offset / BLKSIZE;
		startByte = oftp->offset % BLKSIZE;

		// I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT

		if (lbk < 12) {                    // lbk is a direct block
			blk = mip->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk
		}
		else if (lbk >= 12 && lbk < 256 + 12) {
			//  indirect blocks

			int addressbuf[256];
			get_block(dev, mip->INODE.i_block[12], addressbuf);
			blk = addressbuf[lbk - 12];
		}
		else {
			//  double indirect blocks
			int addressbuf1[256];
			int addressbuf2[256];
			get_block(dev, mip->INODE.i_block[13], addressbuf1);

			get_block(dev, addressbuf1[(lbk - (256 + 12)) / 256], addressbuf2);

			blk = addressbuf2[(lbk - (256 + 12)) % 256];
		}

		/* get the data block into readbuf[BLKSIZE] */
		if (blk == 0) break;
		get_block(mip->dev, blk, readbuf);

		/* copy from startByte to buf[ ], at most remain bytes in this block */
		char *cp = readbuf + startByte;
		remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]
		printf("remain=%d\n", remain);

/*
		while (remain > 0) {
			*cq++ = *cp++;             // copy byte from readbuf[] into buf[]
			oftp->offset++;           // advance offset
			count++;                  // inc count as number of bytes read

			avil--;
			nbytes--;
			remain--;
			if (nbytes <= 0 || avil <= 0)
				break;
		}
*/
		if (remain > 0)
		{
			//void *memcpy(void *dest, const void *src, size_t n);
			//从源src所指的内存地址的起始位置开始拷贝n个字节到目标dest所指的
			//内存地址的起始位置中
			if (nbytes >= remain && avil >= remain)
			{
				memcpy(cq, cp, remain);
				oftp->offset += remain;
				count += remain;
				avil -= remain;
				nbytes -= remain;
				remain = 0;
			}
			else if (nbytes < remain && avil >= remain)
			{
				memcpy(cq, cp, nbytes);
				oftp->offset += nbytes;
				count += nbytes;
				avil -= nbytes;
				nbytes = 0;
			}
			else if (nbytes >= remain && avil < remain)
			{
				memcpy(cq, cp, avil);
				oftp->offset += avil;
				count += avil;
				nbytes -= avil;
				avil = 0;
			}
			else //(nbytes < remain && avil < remain)
			{
				if (avil >= nbytes)
				{
					memcpy(cq, cp, nbytes);
					oftp->offset += nbytes;
					count += nbytes;
					avil -= nbytes;
					nbytes = 0;
				}
				else
				{
					memcpy(cq, cp, avil);
					oftp->offset += avil;
					count += avil;
					nbytes -= avil;
					avil = 0;
				}
			}

		}

		/*
		OPTMIAZATION OF THE READ CODE:

		Instead of reading one byte at a time and updating the counters on each byte,
		TRY to calculate the maximum number of bytes available in a data block and
		the number of bytes still needed to read. Take the minimum of the two, and read
		that many bytes in one operation. Then adjust the counters accordingly. This
		would make the read loops more efficient.

		REQUIRED: optimize the read algorithm in your project.
		*/

		// if one data block is not enough, loop back to OUTER while for more ...
	}
	printf("myread: read %d char from file descriptor %d\n", count, fd);
	printf("exit myread: offset=%d\n", offset);
	return count;   // count is the actual number of bytes read
}

int cat_file()
{
	char mybuf[BLKSIZE], dummy = 0;  // a null char at end of mybuf[ ]
	int n;

	strcpy(newFileName, "0");
	int fd = open_file();
	if (fd < 0) return -1;
	printf("----------------------------------\n");
	while ( n = myread(fd, mybuf, BLKSIZE)) {
		mybuf[n] = 0;             // as a null terminated string
		printf("%s", mybuf);//   <=== THIS works but not good
		//spit out chars from mybuf[ ] but handle \n properly;
	}
	printf("----------------------------------\n");

	close_file(fd);
}