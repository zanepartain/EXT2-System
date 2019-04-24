/************* cp.c file **************/

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
 * Write text to an open fd. If the data block you are writing DNE,
 * then allocate the block, zero it out, and write it back before continuing 
 * to write the text buffer to the open fd.
 */
int mywrite(int fd, char *buf, int nbytes){
    char wbuf[BLKSIZE], *cq = buf;     //cq points at buf[]
    int *iptr, blk, byte_count = 0;    //num bytes written to fd
    OFT *oftp = running->fd[fd];       //get OFT
    MINODE *mip = oftp->mptr;          //point to MINODE of OFT

    /*Write the buf to the fd as long as the fd has space, and
    there are bytes left to write.*/
    while(nbytes > 0){
        int lbk   = oftp->offset / BLKSIZE;
        int start = oftp->offset % BLKSIZE;

        /*Search for lbk in Direct, Indirect, and Double Indirect Blocks.
        If lbk doesnt exist allocate a new block and write it back to disk.*/
        if(lbk < 12){  
            //DIRECT BLOCK
 
            if(mip->INODE.i_block[lbk] == 0){
                mip->INODE.i_block[lbk] = balloc(mip->dev); //allocate Direct DB
            }
 
            blk = mip->INODE.i_block[lbk];  //turn lbk to blk   
        }
        else if(lbk >= 12 && lbk < 256+12){ 
            //INDIRECT BLOCK 

            if(mip->INODE.i_block[12] == 0){
               
                mip->INODE.i_block[12] = balloc(mip->dev);  //allocate i_block[12]
                
                //zero out entire indirect blocks
                get_block(dev, mip->INODE.i_block[12],wbuf);
                for(int i = 0; i < BLKSIZE; i++){
                    wbuf[i] = 0;
                }

                put_block(mip->dev,mip->INODE.i_block[12], wbuf); //write i_block[12] back to disk
            }

            //get indirect block at lbk
            get_block(dev, mip->INODE.i_block[12],wbuf);
            iptr = (int *)wbuf +lbk - 12;
            blk = *iptr;

            //if blk doesnt exist yet, allocate it
            if(blk == 0){
                blk = balloc(mip->dev);
            }
        }
        else{
            //DOUBLE INDIRECT BLOCK

            if(mip->INODE.i_block[13] == 0){
                mip->INODE.i_block[13] = balloc(mip->dev); //allocate i_block[13]

                //zero out entire i_block[13]
                get_block(mip->dev,mip->INODE.i_block[13],wbuf);
                for(int i = 0; i < BLKSIZE; i++){
                    wbuf[i] = 0;
                }

                put_block(mip->dev,mip->INODE.i_block[13],wbuf);  //write i_block[13] back to disk
            }

            get_block(mip->dev,mip->INODE.i_block[13],wbuf);

            int ind_blk = (lbk - 268) / 256;      //get indirect blk num
            int ind_offset = (lbk - 268) % 256;   //get indirect blk offset

            //get ind_blk into blk
            iptr = (int *)wbuf + ind_blk; 
            blk = *iptr;                  

            //if blk doesnt exist yet, allocate it
            if(blk == 0){
                blk = balloc(mip->dev);

                //zero entire ind_blk
                get_block(mip->dev, blk, wbuf);
                for(int i = 0; i < BLKSIZE; i++){
                    wbuf[i] = 0;
                }

                //write ind_blk back to disk
                put_block(mip->dev, blk, wbuf);
            }
            
            //get ind_blk offset block
            get_block(mip->dev, blk, wbuf);
            iptr = (int *)wbuf + ind_offset;
            blk = *iptr;

            //allocate new blk; write back to disk
            if(blk == 0){
                blk = balloc(mip->dev);
                put_block(mip->dev, blk, wbuf);
            }
        }

        /*
            Write to the Data Block
        */
        get_block(mip->dev, blk, wbuf);    //read in disk block to wbuf[]
        char *cp = wbuf + start;           //cp points to start byte in wbuf[]
        int remaining = BLKSIZE - start;   //num of bytes remaining in disk block

        while(remaining > 0){
            //continue to write as long as remaining > 0
            *cp++ = *cq++;

            nbytes--;remaining--; //dec counts
            oftp->offset++;       //advance offset
            byte_count++;

            if(oftp->offset > mip->INODE.i_size){
                mip->INODE.i_size++;
            }
            if(nbytes == 0){
                break;
            }
        }

        put_block(mip->dev, blk, wbuf); //write wbuf[] to disk
    }

    //mark MINODE dirty for iput()
    mip->dirty = 1;
    printf("wrote %d char(s) into fd=%d\n",byte_count,fd);

    return nbytes;
}

