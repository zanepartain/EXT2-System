/************* creat.c file **************/

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
 * Create a new inode as a FILE
 */
MINODE *create_FILE_inode(int ino,int bno,int device){
    MINODE *mip;

    //get MINODE and ptr to INODE
    mip = iget(device, ino);
    INODE *ip = &mip->INODE;

    //Set INODE to new DIR
    ip->i_mode = FILE_MODE_STD;    //0100644
    ip->i_uid  = running->uid;     //Owner uid
    //ip->i_gid  = running->gid;
    ip->i_size = 0;                //no Block
    ip->i_links_count = 1;         //1 link '.'
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 2;

    mip->dirty = 1;  //dirty MINODE
    iput(mip);       //Write INODE back to disk

    return mip;
}


/**
 * Create a FILE mode INODE under the parent MINODE  
 */
int my_creat(MINODE *pmip, char *child_name){
    int ino, bno;
    MINODE *mip;

    ino = ialloc(pmip->dev);     //get free inode#

    printf("ino: %d\n", ino);

    mip = create_FILE_inode(ino,bno,pmip->dev);    //create new inode
    enter_name(pmip,ino,child_name);               //enter new FILE under parent MINODE
}


/**
 * Client version.It will tokenize the pathname into dirname & basename. 
 * Then iff the FILE name doesnt already exist, create the FILE under parent DIR. 
 */
int creat_file(){
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
            my_creat(pmip,basename);
        }
        else{
            //basename already exists
            printf("basename = %s ; ALREADY EXISTS\t",basename);
            printf("dirname = %s \n",dirname);
        }
    }
}