int decFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int incFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int ialloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i = 0; i < ninodes; i++) {
    if (tst_bit(buf, i) == 0) {
      set_bit(buf, i);
      decFreeInodes(dev);

      put_block(dev, imap, buf);

      return i + 1;
    }
  }
  printf("ialloc(): no more free inodes\n");
  return 0;
}

int balloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, bmap, buf);

  for (i = 0; i < nblocks; i++) {
    if (tst_bit(buf, i) == 0) {
      set_bit(buf, i);
      decFreeInodes(dev);

      put_block(dev, bmap, buf);

      return i + 1;
    }
  }
  printf("balloc(): no more free inodes\n");
  return 0;
}

int idealloc(int dev, int ino)//use to be imap
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap
  get_block(dev, imap, buf);

  for (i = ninodes - 1; i >= 0; i--) {
    if (tst_bit(buf, i) == 1) {
      clr_bit(buf, i);
      incFreeInodes(dev);

      put_block(dev, imap, buf);

      return i + 1;
    }
  }
  printf("ialloc(): no more IN_USE inodes\n");
  return 0;
}

int bdealloc(int dev, int bno)//use to be bmap
{
  int  i;
  char buf[BLKSIZE];

  // read block_bitmap
  get_block(dev, bmap, buf);

  for (i = nblocks - 1; i >= 0; i--) {
    if (tst_bit(buf, i) == 1) {
      clr_bit(buf, i);
      incFreeInodes(dev);

      put_block(dev, bmap, buf);

      return i + 1;
    }
  }
  printf("bdealloc(): no more IN_USE blocks\n");
  return 0;
}