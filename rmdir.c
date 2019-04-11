/************* rmdir.c file **************/

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
 * Remove child DIR from parent DIR
 */
int rm_child(MINODE *pmip, char *child_name){

}


/**
 * Prepare to remove a child DIR iff the DIR is empty.
 */
int remove_directory(){
    int ino, pino;
    char my_name[120];
    MINODE *mip, *pmip;

    ino = getino(pathname);   //get inode#
    mip = iget(dev, ino);     //get MINODE by inode#

    if(dir_or_file(mip) == 1 && mip->refCount == 2){
        //mip is a DIR and not BUSY
        if(child_count(mip) == 2){
            //DIR only has two entries '.' & '..' (empty)
            //get parent inode# and INODE
            get_myino(mip,&pino);
            pmip = iget(mip->dev, pino);

            //get name of mip under parent DIR
            get_myname(pmip, ino, my_name);  
            printf("--> %s IS EMPTY\n", my_name);
        }
        else{
            //get parent inode# and INODE
            get_myino(mip,&pino);
            pmip = iget(mip->dev, pino);

            //get name of mip under parent DIR
            get_myname(pmip, ino, my_name); 
            printf("_err: %s NOT EMPTY\n", my_name);
            return;
        }
    }
}