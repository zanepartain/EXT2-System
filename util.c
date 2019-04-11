/*********** util.c file ****************/

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern char   gpath[256];
extern char   *name[64];
extern int    n;
extern int    fd, dev;
extern int    nblocks, ninodes, bmap, imap, inode_start;
extern char   line[256], cmd[32], pathname[256];


int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   


int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}   


int tokenize(char *pathname)
{
  char *t;

  strcpy(gpath,pathname);

  n = 0; //set path count to 0

  t = strtok(gpath,"/");
  while(t){
      name[n] = t;  //append temp name to pathname
      t = strtok(0,"/");
      n++;
  }
}


// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, disp;
  INODE *ip;

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
    
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
      //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev;
       mip->ino = ino;

       // get INODE of ino to buf    
       blk  = (ino-1) / 8 + inode_start;
       disp = (ino-1) % 8;

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


iput(MINODE *mip)
{
 int i, block, offset;
 char buf[BLKSIZE];
 INODE *ip;

 if (mip==0) 
     return;

 mip->refCount--;
 
 if (mip->refCount > 0) return;
 if (!mip->dirty)       return;
 
 /* write back */
 printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino); 

 block =  ((mip->ino - 1) / 8) + inode_start;
 offset =  (mip->ino - 1) % 8;

 /* first get the block containing this inode */
 get_block(mip->dev, block, buf);

 ip = (INODE *)buf + offset;
 *ip = mip->INODE;

 put_block(mip->dev, block, buf);

} 


//
/*search: searches all INODE Blocks in mip, for name Lab6*/
//
int search(MINODE *mip, char *name)
{
  // YOUR serach() fucntion as in LAB 6
  int i, inode_num, blk, offset;
  char sbuf[BLKSIZE], temp[256], dir_name[256];
  char *char_p;

  strcpy(dir_name,name);
  
  for (i=0; i < 12; i++){         // assume DIR at most 12 direct blocks

      if(mip->INODE.i_block[i] == 0){   //never found the dir name
          return 0; 
      }

      // YOU SHOULD print i_block[i] number here
      get_block(mip->dev, mip->INODE.i_block[i], sbuf);

      dp = (DIR *)sbuf;
      char_p = sbuf;

      while(char_p < sbuf + BLKSIZE){
          strncpy(temp, dp->name, dp->name_len);  //make name a string
          temp[dp->name_len] = 0;                    //ensure null at end
          
          if(strcmp(temp,name) == 0){  //found dir, return inode number
              return dp->inode;
          }

          char_p += dp->rec_len;    //advance char_p by rec_len
          dp = (DIR *)char_p;       //pull dir_p to next entry
      }
  }

  return 0;
}


//
/*getino : path2node lab6*/
//
int getino(char *pathname)
{
  int i, ino, blk, disp;
  INODE *ip;
  MINODE *mip;

  printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
      return 2;

  if (pathname[0]=='/')
    mip = iget(dev, 2);
  else
    mip = iget(running->cwd->dev, running->cwd->ino);

  tokenize(pathname);

  for (i=0; i<n; i++){
      printf("===========================================\n");
      ino = search(mip, name[i]);

      if (ino==0){
         iput(mip);
         printf("name %s does not exist\n", name[i]);
         return 0;
      }
      iput(mip);
      mip = iget(dev, ino);
   }
   return ino;
}


/*************************MY UTILITY FUNCTIONS**********************************/
/**********************                         ********************************/

/**
 * Test, Set, and Clear bits in buf 
 */
int test_bit(char *buf, int bit){
    if( buf[bit/8] & (1 << (bit % 8)))
    {
        return 1;
    }
    return 0;
}
int set_bit(char *buf, int bit){
    buf[bit/8] |= (1 << (bit % 8));
}
int clear_bit(char *buf,int bit){
    buf[bit/8] &= ~(1 << (bit % 8));
}


/*
will determine if the MINODE is pointing to a DIR or FILE
*/
int dir_or_file(MINODE *mip){  
  int type = -1;
  
  if ((mip->INODE.i_mode & 0xF000) == 0x8000) // if (S_ISREG())
  {
    type = 0;
  }
  if ((mip->INODE.i_mode & 0xF000) == 0x4000) // if (S_ISDIR())
  {
    type = 1;
  }
  if ((mip->INODE.i_mode & 0xF000) == 0xA000) // if (S_ISLNK())
  {
    type = 2;
  }

  return type;
}


/**
* Decrement or increment the free blocks count in both the
* SUPER block and GROUP_DESCRIPTOR block.
*/
int dec_inc_freeBLOCKS(int dev, char action){
    char   buf[BLKSIZE];
    SUPER *super;
    GD    *group_descriptor;

    get_block(dev,1,buf);  //get super block
    super = (SUPER *)buf;

    switch (action)
    {
        case '+':
            //put super block
            super->s_free_blocks_count++;
            put_block(dev,1,buf);
            
            //get and put group descriptor block
            get_block(dev,2,buf);  
            group_descriptor = (GD *)buf;
            group_descriptor->bg_free_blocks_count++;
            put_block(dev,2,buf);
            break;

        case '-':
            //put super block
            super->s_free_blocks_count--;
            put_block(dev,1,buf);
            
            //get and put group descriptor block
            get_block(dev,2,buf);  
            group_descriptor = (GD *)buf;
            group_descriptor->bg_free_blocks_count--;
            put_block(dev,2,buf);
            break;

        default:
            printf("_err: Invalid action ; dec_inc_freeINODES");
            break;
    }
}


/**
* Decrement or increment the free INODES count in both the
* SUPER block and GROUP_DESCRIPTOR block.
*/
int dec_inc_freeINODES(int dev, char action){
    char   buf[BLKSIZE];
    SUPER *super;
    GD    *group_descriptor;

    get_block(dev,1,buf);  //get super block
    super = (SUPER *)buf;

    switch (action)
    {
        case '+':
            //put super block
            super->s_free_inodes_count++;
            put_block(dev,1,buf);
            
            //get and put group descriptor block
            get_block(dev,2,buf);  
            group_descriptor = (GD *)buf;
            group_descriptor->bg_free_inodes_count++;
            put_block(dev,2,buf);
            break;

        case '-':
            //put super block
            super->s_free_inodes_count--;
            put_block(dev,1,buf);
            
            //get and put group descriptor block
            get_block(dev,2,buf);  
            group_descriptor = (GD *)buf;
            group_descriptor->bg_free_inodes_count--;
            put_block(dev,2,buf);
            break;

        default:
            printf("_err: Invalid action ; dec_inc_freeINODES");
            break;
    }
}


/**
 * allocate new Data Block
 */
int balloc(int device){
    int iter;
    char buf[BLKSIZE];

    //read block_bitmap block
    get_block(device, bmap, buf);

    for(iter = 0; iter < nblocks; iter++){
        if(test_bit(buf,iter) == 0){
            set_bit(buf,iter);
            put_block(device,bmap,buf);
            dec_inc_freeBLOCKS(device, '-');
            return iter+1;
        }
    }
}


/**
 * allocate a new INODE
 */
int ialloc(int device){
    int iter;
    char buf[BLKSIZE];

    //read inode_bitmap block
    get_block(device, imap, buf);

    for(iter = 0; iter < ninodes; iter++){
        if(test_bit(buf,iter) == 0){
            set_bit(buf,iter);
            put_block(device,imap,buf);
            dec_inc_freeINODES(device, '-');
            return iter+1;
        }
    }
}


/**
 * Divides the path into dirname and basename. Returns basename
 */
int dirname_basename(char *dirname, char *basename){

    if(n > 0){
        //set basename as last path name
        strcat(basename,name[n-1]);
    }

    if(pathname[0] == '/'){
        //exact path
        dirname[0] = '/';
    }
    
    for(int i = 0; i < n - 1; i++){
        //append path name to dirname
        strcat(dirname,name[i]);
        strcat(dirname,"/");
    }

    return basename;
}


/**
 * From the parent MINODE, the inode# of a free inode, and the
 * name of new DIR or FILE, create the new DIR or FILE under 
 * the paerent MINODE.
 */
int enter_name(MINODE *pmip, int ino, char *name){
    int i, ideal_length, need_length, remain;
    char *cp, sbuf[BLKSIZE], temp[256];
    INODE *ip = &pmip->INODE;

    need_length = 4*((8 + strlen(name) + 3)/4);  //rec_len needed for the new DIR entry

    //traverse 12 data blocks in INODE
    for(i = 0; i < 12;i++){
        
        if(ip->i_block[i] == 0){
            break;
        }

        //get pmip's data block into sbuf
        get_block(pmip->dev,ip->i_block[i],sbuf);

        dp = (DIR *)sbuf; 
        cp = sbuf;

        while(cp + dp->rec_len < sbuf + BLKSIZE)
        {
            //rec_len (except for last direntry)
            ideal_length = 4*((8 + dp->name_len + 3)/4);  

            cp += dp->rec_len;  //advance char_p by rec_len
            dp = (DIR *)cp;     //pull dir_p to next entry
        }

        //remaining space in Data Block after last entry
        remain = dp->rec_len - ideal_length; 

        if(remain >= need_length)
        {
            //set Last entry rec_len to its ideal length
            dp->rec_len = ideal_length; 

            cp += dp->rec_len;   //advance cp & pull new dir entry
            dp = (DIR *)cp;

            //dp points to a new empty dir entry 
            //where we will enter the new DIR
            dp->rec_len = remain;
            dp->name_len = strlen(name);
            dp->inode = ino;
            strcpy(dp->name,name);
            
            //write the block back to the disk
            put_block(pmip->dev,ip->i_block[i],sbuf);
            iput(pmip);

            return;
        }
        else{
            i++;

            //get pmip's new data block into sbuf
            get_block(pmip->dev,ip->i_block[i],sbuf);

            dp = (DIR *)sbuf;

            pmip->INODE.i_size += BLKSIZE;  //increment parent size by 1024  
            
            //dp points to a new empty data block  
            dp->rec_len = BLKSIZE;
            dp->name_len = strlen(name);
            dp->inode = ino;
            strcpy(dp->name,name);

            //write the block back to the disk
            put_block(pmip->dev,ip->i_block[i],sbuf);
            iput(pmip);

            return;
        }
    }
}


/**
 * Count the number of children a parent DIR has. Return the child count. 
 */
int child_count(MINODE *pmip){
    int count = 0,i;
    char sbuf[BLKSIZE], *cp;
    INODE *ip = &pmip->INODE;

     for(i = 0; i < 12;i++){
        
        if(ip->i_block[i] == 0){
            break;
        }

        //get pmip's data block into sbuf
        get_block(pmip->dev,ip->i_block[i],sbuf);
       
        dp = (DIR *)sbuf; 
        cp = sbuf;

        while(cp < sbuf + BLKSIZE){                  
            count++;
            cp += dp->rec_len;  //advance char_p by rec_len
            dp = (DIR *)cp;     //pull dir_p to next entry
        }
    }
    
    return count;
}

/*************************END MY UTILITY FUNCTIONS**********************************/
/***********************************************************************************/

