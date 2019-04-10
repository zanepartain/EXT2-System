/************* mkdir_rmdir.c file **************/

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

#define OWNER  000700
#define GROUP  000070
#define OTHER  000007

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
 * name of new DIR, create the new DIR under the paerent MINODE.
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
        printf("i_block[%d] %d:\n",i, ip->i_block[i]);
        printf("******************************\n");

        dp = (DIR *)sbuf; 
        cp = sbuf;

        printf("inode# rec_len name_len name\n");
        while(cp + dp->rec_len < sbuf + BLKSIZE){
            //make name a string, ensure ending null
            strncpy(temp, dp->name, dp->name_len);  
            temp[dp->name_len] = 0;                    

            printf("%4d %7d %7d\t%s\n\n", 
            dp->inode, dp->rec_len, dp->name_len, temp);

            ideal_length = 4*((8 + dp->name_len + 3)/4);  //rec_len (except for last direntry)

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
            dp->file_type = DIR_MODE;
            dp->rec_len = remain;
            dp->name_len = strlen(name);
            dp->inode = ino;
            strcpy(dp->name,name);
            
            //write the block back to the disk
            put_block(pmip->dev,ip->i_block[i],sbuf);
            iput(pmip);

            return 1;
        }
    }
}


/**
 * Create a new inode as a DIR
 */
MINODE *create_DIR_inode(int ino,int bno,int device){
    MINODE *mip;

    //get MINODE and ptr to INODE
    mip = iget(device, ino);
    INODE *ip = &mip->INODE;

    //Set INODE to new DIR
    ip->i_mode = DIR_MODE;       //0x41ED
    ip->i_uid  = running->uid;   //Owner uid
    //ip->i_gid  = running->gid;
    ip->i_size = BLKSIZE;        //1024 bits
    ip->i_links_count = 2;       // 2links '.' and '..'
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 2;            // LINUX: Blocks count in 512-byte chunks
    ip->i_block[0] = bno;        //new DIR has 1 data block
    
    for(int i = 1; i <= 14; i++)
    {
        ip->i_block[i] = 0;
    }

    mip->dirty = 1;  //dirty MINODE
    iput(mip);       //Write INODE back to disk

    return mip;
}


/**
 * Create a new data block as a DIR
 */
MINODE *create_DIR_block(int ino,int pino, int bno){
    char buf[BLKSIZE];
    dp = (DIR *)buf;

    __bzero(buf,BLKSIZE);

    //Make '.' entry in DIR block
    dp->inode = ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    dp->name[0] = '.';

    //Make '..' entry in DIR block
    dp = (char *)dp + 12;
    dp->inode = pino;
    dp->rec_len = BLKSIZE - 12;
    dp->name_len = 2;
    dp->name[0] = dp->name[1]  = '.';  
    put_block(dev, bno, buf);
}


/**
 * creates a new INODE and DIR block, then enters the new
 * DIR into the parent MINODE's last DIR entry 
 */
int my_mkdir(MINODE *pmip, char *child_name){
    int ino, bno;
    MINODE *mip;

    ino = ialloc(pmip->dev);     //get free inode#
    bno = balloc(pmip->dev);     //get free block#
    
    printf("ino: %d\n", ino);
    printf("bno: %d\n", bno);

    mip = create_DIR_inode(ino,bno,pmip->dev);      //create new inode
    create_DIR_block(ino,pmip->ino, bno); //create new DIR block
    enter_name(pmip, ino, child_name);    //create new INODE under pmip
}


/**
* Makes a new child DIR inside of the specified parent DIR.
* (iff parent DIR exists)
*/
int make_directory(){
    char dirname[120] = {""}, basename[120] = {""};
    int pino, ppino;
    MINODE *pmip;
    int device;

    tokenize(pathname);  //tokenize pathname

    dirname_basename(dirname,basename);  //get basname and dirname

    /** get parent inode# */
    if(strlen(dirname) == 0)
    {
        //get cwd inode#
        pino = get_myino(running->cwd,&ppino); 
        device = running->cwd->dev;
    }
    else
    {
        //get parent inode# from dirname 
        pino = getino(dirname);   
        device = root->dev;     
    }

    pmip = iget(device,pino);  //get parent MINODE

    //parent MINODE is a DIR
    if(dir_or_file(pmip) == 1){
        if(search(pmip,basename) == 0){
            //basename does not exist
            printf("basename = %s ; DOES NOT EXIST\t",basename);
            printf("dirname = %s \n",dirname);
            my_mkdir(pmip,basename);
        }
        else{
            //basename already exists
            printf("basename = %s ; ALREADY EXISTS\t",basename);
            printf("dirname = %s \n",dirname);
        }
    }
}

