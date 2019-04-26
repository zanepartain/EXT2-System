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
    int ino, mode = 0;
    MINODE *mip;

    //get MINODE of file
    ino = getino(filepath);

    if(ino){
        mip = iget(dev,ino);
    }
    else{
        printf("_err: %s file DNE\n", name[n-1]);
        return;
    }

    //convert string from sourcepath to octal rep
    mode = (int)strtol(sourcepath, (char **)NULL,8);

    //clear old mode and write new permissions
    mip->INODE.i_mode = (mip->INODE.i_mode & 0xF000);
    mip->INODE.i_mode |= mode;
    mip->dirty = 1;

    iput(mip); //write MINODE back
}


/**
 * Touch the specified file by pathname, and update its timestamp
 * to a current timestamp
 */
int mytouch(){
    int ino;
    MINODE *mip;

    ino = getino(pathname); //get file ino#

    if(ino == 0){
        printf("_err: file DNE\n");
        return;
    }

    mip = iget(dev,ino); //get MINODE by ino#

    //change mip date and time to current time
    mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);

    //mark dirty and write mip back
    mip->dirty = 1;
    iput(mip);
}