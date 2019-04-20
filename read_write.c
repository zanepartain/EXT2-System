/************* read_write.c file **************/
#include "type.h";
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
 * Reads n bytes from an opened file descriptor into a buffer
 * area in user space.
 */
int read_file(int fd,char *buf, int nbytes){
    int byte_count = 0;            //bytes read
    OFT *ofd = running->fd[fd];    //get open file descriptor
    MINODE *mip = ofd->mptr;       //get MINODE of open file descriptor
    
    //offset of READ file ; logical block; 
    // available bytes in file ; start READ ; remaining bytes;               
    int offset = ofd->offset;
    int lbk    = offset / BLKSIZE;
    int start  = offset % BLKSIZE;
    int remaining = BLKSIZE - start;
    int available = mip->INODE.i_size - offset;




}