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

