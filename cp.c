/************* cp.c file **************/
#include "type.h"
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
 * Copies the source file to a destination file. If destination
 * file DNE, then it will create a new file.
 */
int mycp(char *source, char *dest){
    char sbuf[BLKSIZE];   //source file buffer[]
    int sino, dino;       //soucre & dest ino
    int lsfd, ldfd;       //source & dest fd
    int nbytes;           //number of bytes read from source
    MINODE *smip, *dmip;   //source & dest MINODE's


    /*Get the source MINODE. Verify it exists, and that it 
    is a REG FILE. Then open source file for READ, then read file*/
    sino = getino(source);

    if(sino == 0){
        printf("_err: (source) %s DNE\n", name[n-1]);
        return;
    }

    //get source MINODE and verify is REG FILE
    smip = iget(dev,sino); 
    if(dir_or_file(smip) != 0){
        printf("_err: %s is not a REG FILE\n", name[n-1]);
        return;
    }

    //open source file for READ ; READ entire file into sbuf[]
    lsfd = open_file(source, "R");  
    nbytes = read_file(lsfd, sbuf, smip->INODE.i_size);


    /*Get the dest MINODE. If doesnt exist ten create it as REG FILE.
    Then open dest file for WRITE, then write  sbuf[] to file*/
    dino = getino(dest); 

    if(dino == 0){
        creat_file(dest);        //create dest file
        ldfd = open(dest, "W");  //open dest file for WRITE
    }
    else{
        //get dest MINODE and verify is REG FILE
        dmip = iget(dev,dino); 
        if(dir_or_file(dmip) != 0){
            printf("_err: (dest) %s is not a REG FILE\n", name[n-1]);
            return;
        }


    }
}