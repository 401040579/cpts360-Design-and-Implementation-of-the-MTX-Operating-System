int my_touch()
{
	char pathnamecp[64];

	strcpy(pathnamecp, pathname);

	int ino = getino(&dev, pathnamecp);

	if (ino == 0)
	{
		printf("file is not here. creat_file()\n");
		creat_file();
	}
	else
	{
		printf("file is here. touch time\n");
		MINODE *mip = iget(dev, ino);

		mip->INODE.i_atime = time(0L);
		mip->INODE.i_ctime = time(0L);
		mip->dirty = 1;
		iput(mip);
	}
}

int my_chmod()
{
	char pathnamecp[64]; char newFileNamecp[64];

	strcpy(pathnamecp, pathname);
	strcpy(newFileNamecp, newFileName);

	int ino = getino(&dev, newFileNamecp);

	if (ino == 0)
	{
		printf("file is not here. creat_file()\n");
		return -1;
	}
	else
	{
		printf("file is here.\n");
		MINODE *mip = iget(dev, ino);
		/*
		参数base范围从2至36，或0。参数base代表采用的进制方式，如base值为10则采用10进制，
		若base值为16则采用16进制等。当base值为0时则是采用10进制做转换，但遇到如’0x’
		前置字符则会使用16进制做转换、遇到’0’前置字符而不是’0x’的时候会使用8进制做转换。
		一开始strtol()会扫描参数nptr字符串，跳过前面的空格字符，直到遇上数字或正负符号才
		开始做转换，再遇到非数字或字符串结束时('\0')结束转换，并将结果返回。若参数endptr
		不为NULL，则会将遇到不合条件而终止的nptr中的字符指针由endptr返回；若参数endptr
		为NULL，则会不返回非法字符串*/

		//1.不仅可以识别十进制整数，还可以识别其它进制的整数，取决于base参数，比如strtol
		//("0XDEADbeE~~", NULL, 16)返回0xdeadbee的值，strtol("0777~~", NULL, 8)返回0777的值。
		long int new_mode = strtol(pathnamecp, NULL, 8);
		mip->INODE.i_mode = (mip->INODE.i_mode & 0xF000) | new_mode;

		mip->dirty = 1;
		iput(mip);

		return 1;

	}
}