/************* stat_chmod_utime.c file **************/

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern char   gpath[256];
extern char   *name[64];
extern int    n;
extern int    fd, dev;
extern int    nblocks, ninodes, bmap, imap, inode_start;
extern char   line[256], cmd[32], pathname[256], sourcepath[256];

#define OWNER  000700
#define GROUP  000070
#define OTHER  000007

#include "string.h"

/**
 * Get the stats on a specified file and put them into the char buffer
 */
struct stat my_stat(char *filepath){
    int ino;
    MINODE *mip;
    struct stat mstat;
    INODE *ip;

    //get MINODE of file
    ino = getino(filepath);
    mip = iget(dev,ino);

    // COPY FILE FIELDS TO STAT
    mstat.st_dev = mip->dev;
    mstat.st_ino = mip->ino;
    mstat.st_size = mip->INODE.i_size;
    mstat.st_ctime = mip->INODE.i_ctime;
    mstat.st_blocks = mip->INODE.i_blocks;
    mstat.st_mode = mip->INODE.i_mode;
    mstat.st_nlink = mip->INODE.i_links_count;
    mstat.st_uid = mip->INODE.i_uid;
    mstat.st_blksize = 0;

    iput(mip); //write MINODE back

    return mstat;
}


/**
 * change the MINODE mode to an executable file
 */
int my_chmod(char *filepath){
    int ino;
    MINODE *mip;

    //get MINODE of file
    ino = getino(filepath);
    mip = iget(dev,ino);

    mip->INODE.i_mode |=  0x1A4;
    mip->dirty = 1;

    iput(mip); //write MINODE back
}