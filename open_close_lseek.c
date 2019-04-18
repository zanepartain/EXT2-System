/************* open_close_lseek.c file **************/

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

#include "string.h"

/**
 * Opens a file for read or write using O_RDONLY, O_WRONLY, O_RDWR.
 * These can be bitwise or-ed with O_CREAT, O_APPEND, O_TRUNC, etc.
 * Return a file descriptor.
 */
int open_file(int mode){
    //mode := 0|1|2|3 for R|W|RW|APPEND
    int ino;
    MINODE *mip;

    ino = getino(pathname); //get file inode#

    if(ino == 0){
        //file doesnt exist; create new file; get inode#
        creat_file(pathname);
        ino = getino(pathname);
    }

    mip = iget(dev,ino); //get MINODE of file
}