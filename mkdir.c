/************* mkdir.c file **************/

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

    mip = create_DIR_inode(ino,bno,pmip->dev);  //create new inode
    create_DIR_block(ino,pmip->ino, bno);       //create new DIR block
    enter_name(pmip, ino, child_name);          //create new INODE under pmip
    pmip->INODE.i_links_count++;                //increment parent DIR link count
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

    dirname_basename(dirname,basename);  //get basename and dirname

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

