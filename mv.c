/************* mv.c file **************/

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

#include <string.h>


/**
 * Move a source file to a destination.
 */
int mymv(char *source, char *dest){
    char sbuf[BLKSIZE];   //source file buffer[]
    int sino, dino;       //soucre & dest ino
    int lsfd, ldfd;       //source & dest fd
    int nbytes;           //number of bytes read from source
    MINODE *smip, *dmip;  //source & dest MINODE's

    /*Get source ino and verify it exists*/
    sino = getino(source);

    if(sino == 0){
        printf("_err: (source) %s DNE\n", name[n-1]);
        return;
    }

    smip = iget(dev,sino); //source MINODE

    strcat(dest,"/");
    strcat(dest,name[n-1]);

    mylink(); //link source and dest

    unlink(); //unlink source from parent

    return;
}