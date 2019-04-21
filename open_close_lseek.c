/************* open_close_lseek.c file **************/
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

#include "string.h"


/**
 * Truncate the all of the MINODE's Data blocks, by deallocating them.
 * Then set the MINODE size to 0
 */
int truncate(MINODE *mip){
  int i, inode_num, blk, offset;
  char sbuf[BLKSIZE];
  char *char_p;
  
    for(int i = 0; i < 12; i++){
        if(mip->INODE.i_block[i] == 0){
            continue;
        }
        //deallocate the BLOCK
        bdalloc(mip->dev, mip->INODE.i_block[i]);
    }

    //deallocate indirect blocks
    get_block(dev, mip->INODE.i_block[12],sbuf);
    int *idp = (int *)sbuf;

     while(*idp && idp < sbuf + BLKSIZE){
         bdalloc(dev,idp);
         idp++;
     }

    //deallocate double indirect blocks
    get_block(dev, mip->INODE.i_block[13],sbuf);
    int *didp = (int *)sbuf;

     while(*didp && didp < sbuf + BLKSIZE){
         bdalloc(dev,didp);
         didp++;
     }

     mip->INODE.i_size = 0; 

  return 0;
}

/**
 * Opens a file for read or write using O_RDONLY, O_WRONLY, O_RDWR.
 * These can be bitwise or-ed with O_CREAT, O_APPEND, O_TRUNC, etc.
 * Return a file descriptor.
 */
int open_file(){
    //mode := 0|1|2|3 for R|W|RW|APPEND
    char  buf[256];
    char mode[2];
    int ino, index = -1;
    MINODE *mip;
    struct stat mstat;

    printf("Enter pathname and mode: ");
    fgets(buf, 128, stdin);
    buf[strlen(buf)-1] = 0;
    if (buf[0]==0)
    {
        return; //error
    }
    sscanf(buf,"%s %s", pathname, mode);
    

    ino = getino(pathname); //get file inode#

    if(ino == 0){
        //file doesnt exist; create new file; get inode#
        creat_file(pathname);
        ino = getino(pathname);
    }

    mip = iget(dev,ino); //get MINODE of file

    if(dir_or_file(mip) == 0){
        //is a REG FILE
        //create new open file table instance
        OFT oft;
        oft.mode = atoi(mode);
        oft.mptr = mip;
        oft.refCount = 1;

        //if mode:=APPEND
        if(mode == 3){
            oft.offset = mip->INODE.i_size;
        }
        else{
            oft.offset = 0;
        }
        
        //search for first free fd[index] entry 
        for(int i = 0; i < NFD; i++){
            //free entry 
            if(running->fd[i] == 0){
                index = i;
                break;
            }
        }

        if(index != -1){
            //insert new oft entry to running PROC
            running->fd[index] = &oft;
            printOFT(oft);
        }
        else{
            //error
            printf("_err: No FREE fd entries available\n");
        }
    }
    else{
        //error
        printf("_err: %s is not a REG FILE\n",name[n-1]);
    }

    return index; //return the index (-1) if error
}


/**
 * Close an open File Descriptor. Ensure that it is infact open,
 * then close it in the current running PROC.
 */
int close_file(int fd){

    //check for valid fd
    if(fd < 0 && fd >= NFD){
        printf("_err: INVALID fd\n");
        return -1;
    }

    if(running->fd[fd] != 0){
        //points to an OFT; decrement refCount;
        OFT *oftp = running->fd[fd];
        oftp->refCount--;
        printOFT((*oftp));
        if(oftp->refCount == 0){
            iput(oftp->mptr); 
        }

        
    }

    //clear OFT at index fd
    running->fd[fd] = 0; 
    return 0;
}


/**
 * Set the offset of the open File Descriptor to the
 * new offset position. Then return the old original offset.
 */
int my_lseek(int fd, int position){
    int orig_offset = -1;

    //check for valid fd
    if(fd < 0 && fd >= NFD){
        printf("_err: INVALID fd\n");
        return -1;
    }

    if(running->fd[fd] != 0){
        OFT *oftp = running->fd[fd];
        orig_offset  = oftp->offset;
        
        //ensure the position is not out of bounds of file
        if(position >= 0 && position < oftp->mptr->INODE.i_size){
            oftp->offset = position;
        }
        else{
            //error
            printf("_err: new offset position is out of bounds\n");
        }
    }

    return orig_offset;
}