int my_rmdir()
{
	//1. Extract cmd, pathname from line and save them as globals.
	printf("[rmdir] 1. Extract cmd, pathname from line and save them as global.\n");
	printf("line=%s cmd=%s pathname=%s\n", line, cmd, pathname);

	//2. get inumber of pathname: determine dev, then
	int ino = getino(&dev, pathname);
	printf("[rmdir] 2. get inumber of pathname: determine dev, then\n");
	printf("ino=%d\n", ino);

	//3. get its minode[ ] pointer:
	MINODE *mip = iget(dev, ino);
	printf("[rmdir] 3. get its minode[ ] pointer:\n");
	printf("mip->dev=%d mip->ino=%d\n", mip->dev, mip->ino);

	// 4. check ownership
	// super user : OK
	// not super user: uid must match
	if (running->uid == 0) printf("[rmdir] super user : OK\n");
	else
	{
		printf("[rmdir] not super user: uid must match\n");
		if (running->uid != mip->INODE.i_uid)
		{
			printf("[rmdir] uid doesn't match\n");
			printf("running->uid=%d mip->INODE.uid=%d\n",
			       running->uid, mip->INODE.i_uid);
			iput(mip);
			return -1;
		}
		printf("[rmdir] uid match\n");
	}

	/*
	 5. check DIR type (HOW?) AND not BUSY (HOW?) AND is empty:

	 HOWTO check whether a DIR is empty:
	 First, check link count (links_count > 2 means not empty);
	 However, links_count == 2 may still have FILEs, so go through its data
	 block(s) to see whether it has any entries in addition to . and ..

	 if (NOT DIR || BUSY || not empty): iput(mip); retunr -1;
	*/

	if (!S_ISDIR(mip->INODE.i_mode))
	{
		printf("[rmdir] is not dir\n");
		iput(mip);
		return -1;
	}

	if (mip->refCount != 1)
	{
		printf("[rmdir] is BUSY\n");
		printf("mip->refCount=%d\n", mip->refCount);
		iput(mip);
		return -1;
	}

	if (mip->INODE.i_links_count > 2)
	{
		printf("[rmdir] some dirs in the dir\n");
		printf("[rmdir] links_count=%d\n", mip->INODE.i_links_count);
		iput(mip);
		return -1;
	}

	printf("[rmdir]Checking FILEs\n");
	char *cp;
	char buf[BLKSIZE];

	int bno = mip->INODE.i_block[0];

	get_block(dev, bno, buf);
	cp = buf;
	dp = (DIR *) buf;

	printf("[rmdir] Checking .\n");
	printf("dp->name=%s\n", dp->name);
	if (strcmp(dp->name, ".") != 0)
	{
		iput(mip);
		return -1;
	}

	cp += dp->rec_len;
	dp = (DIR *)cp;
	printf("[rmdir] Checking ..\n");
	printf("dp->name=%s\n", dp->name);
	if (strcmp(dp->name, "..") != 0)
	{
		iput(mip);
		return -1;
	}

	printf("[rmdir] Checking ..'s rec_len=%d\n", dp->rec_len);
	int pino = dp->inode;
	if (dp->rec_len != 1012)
	{
		printf("some things in the dir.\n");
		iput(mip);
		return -1;
	}

	printf("[rmdir] Checking DONE\n");
	put_block(dev, bno, buf);
	//return 1;

	/*
	6. ASSUME passed the above checks.
	 Deallocate its block and inode
	*/
	printf("[rmdir] 6. Deallocate its block and inode\n");
	for (int i = 0; i < 12; i++) {
		if (mip->INODE.i_block[i] == 0)
			continue;
		bdealloc(mip->dev, mip->INODE.i_block[i]);
	}
	idealloc(mip->dev, mip->ino);
	int pdev = mip->dev;
	iput(mip); //(which clears mip->refCount = 0);

	printf("[rmdir] 7. get parent DIR's ino and Minode (pointed by pip)\n");
	MINODE *pmip = iget(pdev, pino);

	printf("[rmdir] 8. remove child's entry from parent directory by\n");
	//rm_child(MINODE *pip, char *name);
	char *temp;
	char child[64], pathnamecp[64];
	strcpy(pathnamecp, pathname);
	temp = basename(pathnamecp);
	strcpy(child, temp);
	printf("pathname=%s child=%s\n", pathname, child);

	rm_child(pmip, child);

	pmip->INODE.i_links_count--;
	pmip->INODE.i_atime = pmip->INODE.i_mtime = time(0L);
	pmip->dirty = 1;
	iput(pmip);
	return 1;
}
// rm_child(): removes the entry [INO rlen nlen name] from parent's data block.
int rm_child(MINODE *pmip, char *name)
{
	char buf[BLKSIZE];
	char *cp;
	for (int i = 0; i < 12; i++)
	{
		int bno = pmip->INODE.i_block[i];
		DIR *pre_dp;
		if (bno == 0) break;
		get_block(pmip->dev, bno, buf);
		cp = buf;
		dp = (DIR *) cp;
		char namebuf[64];
		printf("[rm_child] bno=%d dp->name=%s\n", bno, dp->name);
		//strcpy(namebuf, "");
		namebuf[0] = 0;
		strncpy(namebuf, dp->name, dp->name_len);
		namebuf[dp->name_len] = 0;

		if (strcmp(namebuf, name) == 0)
		{
			printf("[rm_child] It is the FIRST entry\n");
			//do something
			for (int j = i; j < 12; j++)
			{
				pmip->INODE.i_block[j] = pmip->INODE.i_block[j + 1];
			}
			pmip->INODE.i_size -= BLKSIZE;
			put_block(pmip->dev, bno, buf);
			return 1;
		}

		while (cp + dp->rec_len < buf + BLKSIZE)
		{
			//strcpy(namebuf, "");
			namebuf[0] = 0;
			strncpy(namebuf, dp->name, dp->name_len);
			namebuf[dp->name_len] = 0;

			if (strcmp(namebuf, name) == 0)
			{
				printf("[rm_child] It is in the middle\n");

				//void *memcpy(void *dest, const void *src, size_t n);
				//从源src所指的内存地址的起始位置开始拷贝n个字节到目标dest所指的
				//内存地址的起始位置中
				int deleted_rec_len = dp->rec_len;
				char *dest = cp;
				char *src = cp + dp->rec_len;
				printf("dest=%d src=%d\n", dest, src);
				int n = 0;
				while (cp + dp->rec_len < buf + BLKSIZE)
				{	n += dp->rec_len;
					cp += dp->rec_len;
					dp = (DIR *)cp;
				}
				n += dp->rec_len;
				dp->rec_len += deleted_rec_len;
				printf("dp->name=%s\n", dp->name);
				printf("dest=%d src=%d\n", dest, src);
				memmove(dest, src, n);

				printf("Done for remove middle\n");
				put_block(pmip->dev, bno, buf);
				return 1;

			}
			pre_dp = dp;
			
			cp += dp->rec_len;
			dp = (DIR *)cp;
		}
		//strcpy(namebuf, "");
		namebuf[0] = 0;
		strncpy(namebuf, dp->name, dp->name_len);
		namebuf[dp->name_len] = 0;

		if (strcmp(namebuf, name) == 0)
		{
			printf("[rm_child] It is the LAST entry\n");
			printf("dp->name=%s need to remove\n", dp->name);
			//do something;
			int last_ren_len = dp->rec_len;
			printf("%s's ren_len=%d\n", dp->name, last_ren_len);
			printf("%s's rec_len=%d\n", pre_dp->name, pre_dp->rec_len);
			cp = cp - pre_dp->rec_len;
			dp = (DIR *)cp;
			printf("should be new last dp name %s\n", dp->name);
			dp->rec_len += last_ren_len;
			printf("%s's rec_len=%d\n", dp->name, dp->rec_len);
			put_block(pmip->dev, bno, buf);
			return 1;
		}
	}

	printf("[rm_dir]i_block[12]\n");
	int bno = pmip->INODE.i_block[12];

	if (bno != 0)
	{
		DIR *pre_dp;
		int buf1[256];
		get_block(pmip->dev, bno, buf1);

		for (int i = 0; i < 256; i++)
		{
			if (buf1[i] == 0) break;
			get_block(pmip->dev, buf1[i], buf);

			cp = buf;
			dp = (DIR *) cp;
			char namebuf[64];
			printf("[rm_child] bno=%d dp->name=%s\n", bno, dp->name);

			strcpy(namebuf, "");
			strncpy(namebuf, dp->name, dp->name_len);
			namebuf[dp->name_len] = 0;

			if (strcmp(namebuf, name) == 0)
			{
				printf("[rm_child] It is the FIRST entry\n");
				//do something
				for (int j = i; j < 12; j++)
				{
					pmip->INODE.i_block[j] = pmip->INODE.i_block[j + 1];
				}
				pmip->INODE.i_size -= BLKSIZE;
				put_block(pmip->dev, bno, buf);
				return 1;
			}

			while (cp + dp->rec_len < buf + BLKSIZE)
			{
				strcpy(namebuf, "");
				strncpy(namebuf, dp->name, dp->name_len);
				namebuf[dp->name_len] = 0;

				if (strcmp(namebuf, name) == 0)
				{

					printf("[rm_child] It is in the middle\n");

					//void *memcpy(void *dest, const void *src, size_t n);
					//从源src所指的内存地址的起始位置开始拷贝n个字节到目标dest所指的
					//内存地址的起始位置中
					int deleted_rec_len = dp->rec_len;
					char *dest = cp;
					char *src = cp + dp->rec_len;
					printf("dest=%d src=%d\n", dest, src);
					int n = 0;
					while (cp + dp->rec_len < buf + BLKSIZE)
					{	n += dp->rec_len;
						cp += dp->rec_len;
						dp = (DIR *)cp;
					}
					n += dp->rec_len;
					dp->rec_len += deleted_rec_len;
					printf("dp->name=%s\n", dp->name);
					printf("dest=%d src=%d\n", dest, src);
					memmove(dest, src, n);

					printf("Done for remove middle\n");
					put_block(pmip->dev, bno, buf);
					return 1;

				}
				pre_dp = dp;

				cp += dp->rec_len;
				dp = (DIR *)cp;
			}
			strcpy(namebuf, "");
			strncpy(namebuf, dp->name, dp->name_len);
			namebuf[dp->name_len] = 0;

			if (strcmp(namebuf, name) == 0)
			{
				printf("[rm_child] It is the LAST entry\n");
				printf("dp->name=%s need to remove\n", dp->name);
				//do something;
				int last_ren_len = dp->rec_len;
				printf("%s's ren_len=%d\n", dp->name, last_ren_len);
				printf("%s's rec_len=%d\n", pre_dp->name, pre_dp->rec_len);
				cp = cp - pre_dp->rec_len;
				dp = (DIR *)cp;
				printf("should be new last dp name %s\n", dp->name);
				dp->rec_len += last_ren_len;
				printf("%s's rec_len=%d\n", dp->name, dp->rec_len);
				put_block(pmip->dev, bno, buf);
				return 1;
			}
		}
	}

	printf("[rm_dir]i_block[13]\n");
	bno = pmip->INODE.i_block[13];

	if (bno != 0)
	{
		DIR *pre_dp;
		int buf1[256], buf2[256];
		get_block(pmip->dev, bno, buf1);

		for (int i = 0; i < 256; i++)
		{
			if (buf1[i] == 0) break;

			get_block(pmip->dev, buf1[i], buf2);

			for (int j = 0; j < 256; j++)
			{
				get_block(pmip->dev, buf2[j], buf);

				cp = buf;
				dp = (DIR *) cp;
				char namebuf[64];
				printf("[rm_child] bno=%d dp->name=%s\n", bno, dp->name);
				strcpy(namebuf, "");
				strncpy(namebuf, dp->name, dp->name_len);
				namebuf[dp->name_len] = 0;

				if (strcmp(namebuf, name) == 0)
				{
					printf("[rm_child] It is the FIRST entry\n");
					//do something
					for (int j = i; j < 12; j++)
					{
						pmip->INODE.i_block[j] = pmip->INODE.i_block[j + 1];
					}
					pmip->INODE.i_size -= BLKSIZE;
					put_block(pmip->dev, bno, buf);
					return 1;
				}

				while (cp + dp->rec_len < buf + BLKSIZE)
				{
					strcpy(namebuf, "");
					strncpy(namebuf, dp->name, dp->name_len);
					namebuf[dp->name_len] = 0;

					if (strcmp(namebuf, name) == 0)
					{

						printf("[rm_child] It is in the middle\n");

						//void *memcpy(void *dest, const void *src, size_t n);
						//从源src所指的内存地址的起始位置开始拷贝n个字节到目标dest所指的
						//内存地址的起始位置中
						int deleted_rec_len = dp->rec_len;
						char *dest = cp;
						char *src = cp + dp->rec_len;
						printf("dest=%d src=%d\n", dest, src);
						int n = 0;
						while (cp + dp->rec_len < buf + BLKSIZE)
						{	n += dp->rec_len;
							cp += dp->rec_len;
							dp = (DIR *)cp;
						}
						n += dp->rec_len;
						dp->rec_len += deleted_rec_len;
						printf("dp->name=%s\n", dp->name);
						printf("dest=%d src=%d\n", dest, src);
						memmove(dest, src, n);

						printf("Done for remove middle\n");
						put_block(pmip->dev, bno, buf);
						return 1;

					}
					pre_dp = dp;

					cp += dp->rec_len;
					dp = (DIR *)cp;
				}
				strcpy(namebuf, "");
				strncpy(namebuf, dp->name, dp->name_len);
				namebuf[dp->name_len] = 0;

				if (strcmp(namebuf, name) == 0)
				{
					printf("[rm_child] It is the LAST entry\n");
					printf("dp->name=%s need to remove\n", dp->name);
					//do something;
					int last_ren_len = dp->rec_len;
					printf("%s's ren_len=%d\n", dp->name, last_ren_len);
					printf("%s's rec_len=%d\n", pre_dp->name, pre_dp->rec_len);
					cp = cp - pre_dp->rec_len;
					dp = (DIR *)cp;
					printf("should be new last dp name %s\n", dp->name);
					dp->rec_len += last_ren_len;
					printf("%s's rec_len=%d\n", dp->name, dp->rec_len);
					put_block(pmip->dev, bno, buf);
					return 1;
				}
			}
		}
	}
}