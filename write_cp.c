/************* write_cp.c file **************/
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
 * Prompt user for an fd to write to. Verify it is infact an open fd with RW || WR
 * mode. Then prompt the user for an input text. Pass the fd, text, and nbytes to mywrite
 * then return the actual number of bytes written to fd.
 */
int write_file(){
    int lfd, nbytes;
    char buf[BLKSIZE];

    /*Get user input for fd, and text*/
    printf("Enter a fd: ");
    scanf("%d%*c", &lfd);

    //confirm fd exists && fd open for WR || RW
    if(lfd < 0 || lfd >= NFD || running->fd[lfd] == 0){
        printf("_err: invalid file descriptor\n");
        return;
    }
    else if(running->fd[lfd]->mode != 1 && running->fd[lfd]->mode != 2){
        printf("_err: FILE not open for RW or WR\n");
        return;
    }
    else{
        //get text into buf[]
        printf("\nEnter text: ");
        fgets(buf,BLKSIZE,stdin);
        buf[strlen(buf)-1] = 0; 

        nbytes = strlen(buf); //size of file in bytes

        /*print the write data*/
        printf("-----------WRITE------------\n");
        printf("fd = %d, mode=%d\n",lfd, running->fd[lfd]->mode);
        printf("nbytes = %d\n",nbytes);
        printf("text = %s\n",buf);
        printf("---------end WRITE----------\n");
    }

    return(mywrite(lfd,buf,nbytes));
}


/**
 * 
 */
int mywrite(int fd, char *buf, int nbytes){
    char sbuf[BLKSIZE];
    int blk, byte_count = 0;       //num bytes written to fd
    OFT *oftp = running->fd[fd];   //get OFT
    MINODE *mip = oftp->mptr;      //point to MINODE of OFT

    /*Write the buf to the fd as long as the fd has space, and
    there are bytes left to write.*/
    while(nbytes > 0){
        int lbk   = oftp->offset / BLKSIZE;
        int start = oftp->offset % BLKSIZE;

        if(lbk < 12){  
            //DIRECT BLOCK
 
            if(mip->INODE.i_block[lbk] == 0){
                //allocate DB if no DB
                mip->INODE.i_block[lbk] = balloc(mip->dev); 
            }

            //turn lbk to blk   
            blk = mip->INODE.i_block[lbk]; 
        }
        else if(lbk >= 12 && lbk < 256+12){ 
            //INDIRECT BLOCK 

            if(mip->INODE.i_block[12] == 0){
                //allocate new block
                mip->INODE.i_block[12] = balloc(mip->dev);
                
                //zero out entire indirect blocks
                get_block(dev, mip->INODE.i_block[12],sbuf);
                for(int i = 0; i < BLKSIZE; i++){
                    sbuf[i] = 0;
                }

                //write i_block[12] back to disk
                put_block(mip->dev,mip->INODE.i_block[12], sbuf);
            }

            get_block(dev, mip->INODE.i_block[12],sbuf);
            blk = sbuf[lbk - 12];

            //if DB doesnt exist yet, allocate it
            if(blk == 0){
                blk = balloc(mip->dev);
            }
        }
        else{
            //DOUBLE INDIRECT BLOCK

            if(mip->INODE.i_block[13] == 0){
                //allocate new data block
                mip->INODE.i_block[13] = balloc(mip->dev);

                //zero out entire 13th block
                get_block(mip->dev,mip->INODE.i_block[13],sbuf);
                for(int i = 0; i < BLKSIZE; i++){
                    sbuf[i] = 0;
                }

                //write i_lock[13] back to disk
                put_block(mip->dev,mip->INODE.i_block[13],sbuf);
            }

            get_block(mip->dev,mip->INODE.i_block[13],sbuf);

            int id_blk = (lbk - 268) / 256;      //get indirect blk num
            int ind_offset = (lbk - 268) % 256;  //get indirect blk offset

            blk = sbuf[id_blk];                  //get id_blk into blk

            if(blk == 0){
                //no blk yet; so allocate blk
                blk = balloc(mip->dev);

                //zero entire ind_blk
                get_block(mip->dev, blk, sbuf);
                for(int i = 0; i < BLKSIZE; i++){
                    sbuf[i] = 0;
                }

                //write ind_blk back to disk
                put_block(mip->dev, blk, sbuf);
            }

            get_block(mip->dev, blk, sbuf);
            blk = sbuf + ind_offset;

            if(blk == 0){
                //allocate new blk; write back to disk
                blk = balloc(mip->dev);
                put_block(mip->dev, blk, sbuf);
            }
        }

        get_block(mip->dev, blk, sbuf);
        
    }
}
