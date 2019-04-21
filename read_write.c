/************* read_write.c file **************/

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
#include <stdbool.h>


/**
 * Reads n bytes from an opened file descriptor into a buffer
 * area in user space.
 */
int read_file(int fd,char *buf, int nbytes){
    char sbuf[BLKSIZE], readbuf[BLKSIZE];
    int byte_count = 0, blk = -1;      //bytes read & blk number
    char *cbuf = buf;                    //cbuf points to buf
    OFT *ofd = running->fd[fd];        //get open file descriptor
    MINODE *mip = ofd->mptr;           //get MINODE of open file descriptor
    
    if(ofd->mode != 0 && ofd->mode != 2){
        printf("_err: FILE not open for RD or RW\n");
        return;
    }

    //offset of READ file 
    // available bytes in file
    int offset = ofd->offset;
    int available = mip->INODE.i_size - offset;

    while(nbytes && available){
        int lbk    = offset / BLKSIZE;
        int start  = offset % BLKSIZE;

        if(lbk < 12){
            //DIRECT BLOCK
            //turn to physical block
            blk = mip->INODE.i_block[lbk];
        }
        else if (lbk >= 12 && lbk < 256+12){
            //INDIRECT BLOCK
            get_block(dev,mip->INODE.i_block[12],sbuf);
            int *idp = (int *)sbuf;
            
            while(*idp && idp < sbuf + BLKSIZE){
                //turn to physical block;
                if(*idp == lbk){
                    blk = lbk; 
                    break;
                }
                idp++;
            }

        }
        else{
            //DOUBLE INDIRECT BLOCK
            get_block(dev, mip->INODE.i_block[13], sbuf);
            int *didp = (int *)sbuf;
            bool found = false;

            while(*didp && didp < sbuf+BLKSIZE && !found){
                char tbuf[BLKSIZE];
                get_block(dev,*didp,tbuf);
                int *idp = (int *)tbuf;

                ///check for lkb in indirect blks
                while(*idp && idp < tbuf+BLKSIZE && !found){
                    if(*idp == lbk){
                        //turn to physical block
                        blk = lbk; 
                        found = true;
                    }
                    idp++;
                } 

                didp++; //advance DIB
            }
        }

        /*Get data blk into readbuf*/
        get_block(mip->dev, blk, readbuf);  

        char *cp = readbuf + start;
        int remaining = BLKSIZE - start;

        while(remaining > 0){
            *cbuf = *cp++;     //copy byte from readbuf[] into buf[]
            ofd->offset++;     //advance offset
            byte_count++;      //inc byte count
            available--;       //dec the following
            nbytes--;
            remaining--;
            if(nbytes <= 0 || available <= 0)
            {
                break;
            }
        }

    }//end of while(nbytes && available)
    printf("myread: read %d char from file descriptor %d\n", byte_count, fd);
    return byte_count;
}
