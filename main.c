/****************************************************************************
*                   ZANE PARTAIN EXT2-File System                           *
*                                                                           *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h>

#include "type.h"

//GLOBALS
MINODE minode[NMINODE];         //in memory INODES
MTABLE mtable[NMTABLE];         //mount tables
MINODE *root;                   //root MINODE
PROC   proc[NPROC], *running;   //PROC structs, current executing PROC

char   gpath[256]; // global for tokenized components
char   *name[64];  // assume at most 64 components in pathname
int    n;          // number of component strings

int    fd, dev;
int    nblocks, ninodes, bmap, imap, inode_start;
char   line[256], cmd[32], pathname[256], sourcepath[256];

#include "util.c"
#include "cd_ls_pwd.c"
#include "mkdir.c"
#include "creat.c"
#include "rmdir.c"
#include "link_unlink.c"
#include "symlink_readlink.c"
#include "stat_chmod_utime.c"
#include "open_close_lseek.c"
#include "read_write.c"


int init()
{
  int i, j;
  MINODE *mip;
  MTABLE *mtp;
  PROC   *p;
  
  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = 0;
    p->cwd = 0;
    p->status = FREE;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
  }
  for(i = 1; i < NMTABLE; i++){
    mtp = &mtable[i];
    mtp->dev = 0;
  }
}


// load root INODE and set root pointer to it
int mount_root()
{  
  printf("mount_root()\n");
  root = iget(dev, 2);
}


char *disk = "mydisk";
int main(int argc, char *argv[ ])
{
  int ino;
  char buf[BLKSIZE];
  if (argc > 1)
    disk = argv[1];


  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);  exit(1);
  }
  dev = fd;

  /********** read super block at 1024 ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system *****************/
  if (sp->s_magic != SUPER_MAGIC){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  inode_start = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

  //setting the root mtable fields 
  mp = &mtable[0];   
  mp->bmap = bmap;
  mp->imap = imap;
  mp->iblock = inode_start;
  mp->ninodes = ninodes;
  mp->nblocks = nblocks;
  mp->dev = dev;

  init();  
  mount_root();

  printf("root refCount = %d\n", root->refCount);
  
  printf("creating P0 as running process\n");
  running = &proc[0];
  running->status = READY;
  running->cwd = iget(dev, 2);
  
  printf("root refCount = %d\n", root->refCount);

  //printf("hit a key to continue : "); getchar();
  while(1){
    printf("input command : [ls|cd|pwd|mkdir|rmdir|creat|link|unlink|symlink|readlink|quit] ");
    fgets(line,128,stdin);
    line[strlen(line)-1] = 0; 
    if (line[0]==0)
      continue;
    pathname[0] = 0;
    cmd[0] = 0;
    sourcepath[0] = 0;
    
    sscanf(line, "%s %s %s", cmd, pathname,sourcepath);
    printf("cmd=%s pathname=%s\n", cmd, pathname);
    
    char buf[BLKSIZE] = {""};

    if (strcmp(cmd, "ls")==0)
       list_file();
    if (strcmp(cmd, "cd")==0)
       change_dir();
    if (strcmp(cmd, "pwd")==0)
       pwd(running->cwd);
    if (strcmp(cmd,"mkdir")==0)
       make_directory();
    if (strcmp(cmd,"creat")==0)
       creat_file(pathname);
    if (strcmp(cmd,"rmdir")==0)
       remove_directory();
    if (strcmp(cmd,"link")==0)
       link();
    if (strcmp(cmd,"unlink")==0)
       unlink();
    if (strcmp(cmd,"symlink")==0)
       symlink();
    if (strcmp(cmd,"readlink")==0)
       readlink(pathname,buf);
    if (strcmp(cmd,"stat")==0)
       my_stat(pathname);
    if (strcmp(cmd,"chmod")==0)
       my_chmod(pathname);
   if (strcmp(cmd,"open")==0)
       fd = open_file(pathname,sourcepath);
   if (strcmp(cmd,"close")==0)
       close_file(fd);
   if (strcmp(cmd,"read")==0)
       read_file(fd,buf,1024); 

    if (strcmp(cmd, "quit")==0)
       quit();
  }
}
 

int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}
