/************* rmdir.c file **************/

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
 * Deallocate the desired INODE and BLOCK (mip)
 */
int free_INODE_BLOCK(MINODE *mip){
    for(int i = 0; i < 12; i++){
        if(mip->INODE.i_block[i] == 0){
            
            continue;
        }
        //deallocate the BLOCK
        bdalloc(mip->dev, mip->INODE.i_block[i]);
    }

    //deallocate the INODE
    idalloc(mip->dev, mip->ino);
    mip->refCount--;
    
    iput(mip);  //clear mip->refCount = 0
}


/**
 * Remove child DIR from parent DIR
 */
int rm_child(MINODE *pmip, char *child_name){
    int i, ideal_length, location;
    char sbuf[BLKSIZE], temp[256], dir_name[256];
    char *char_p;
    DIR *prev_dp, *cur_dp;

    strcpy(dir_name,child_name);
  
    for (i=0; i < 12; i++){

      if(pmip->INODE.i_block[i] == 0){   //never found the dir name
          continue; 
      }

      // YOU SHOULD print i_block[i] number here
      get_block(pmip->dev, pmip->INODE.i_block[i], sbuf);

      dp = (DIR *)sbuf;
      char_p = sbuf;

      while(char_p < sbuf + BLKSIZE){
          strncpy(temp, dp->name, dp->name_len);   //make name a string
          temp[dp->name_len] = 0;                  //ensure null at end
          
          if(strcmp(temp,child_name) == 0){  
            //found dir to delete
            //rec_len (except for last direntry)
            ideal_length = 4*((8 + dp->name_len + 3)/4);  
            break;
          }
          prev_dp = (DIR *)char_p;  //keep track of previous entry
          char_p += dp->rec_len;    //advance char_p by rec_len
          dp = (DIR *)char_p;       //pull dir_p to next entry
      }

      //Handle deleting the child DIR entry
      if(dp->rec_len == BLKSIZE){
        //FIRST & ONLY entry in parent DIR
        //deallocate data block
        bdalloc(pmip->dev, pmip->INODE.i_block[i]);

        pmip->INODE.i_size -= BLKSIZE; //decrease parent file size

        for(; i < 12; i++){
            //compact parent i_blocks
            get_block(pmip->dev, pmip->INODE.i_block[i+1], sbuf);
            put_block(pmip->dev, pmip->INODE.i_block[i], sbuf);
        }

      }
      else if(dp->rec_len > ideal_length){
        //DIR to delete is last entry
        //absorb last entry rec_len
        prev_dp->rec_len += dp->rec_len;

        //write block back to disk
        put_block(pmip->dev, pmip->INODE.i_block[i], sbuf);
      }
      else{
        //DIR is in middle or multiple entries
        //advance to next entry
        char *cp = char_p;

        cp += dp->rec_len;    //point to entry after dp in Block
        cur_dp = (DIR *)cp;   
        int size = 0;

        //advance to last entry to get total size (bits)
        while(cp + cur_dp->rec_len < sbuf + BLKSIZE){
            cp += cur_dp->rec_len;    
            size += cur_dp->rec_len;
            cur_dp = (DIR *)cp; 
        }

        cur_dp->rec_len += dp->rec_len;  // + deleted rec_len to last entry    
        char_p += dp->rec_len;           // advance char_p to entry after dp
        size += dp->rec_len;             // + deleted rec_len to total size (bits)

        memmove(dp,char_p,size); //move everything left

        //write block back to disk
        put_block(pmip->dev, pmip->INODE.i_block[i], sbuf);
      }
  }
}


/**
 * Prepare to remove a child DIR iff the DIR is empty.
 */
int remove_directory(){
    int ino, pino;
    char my_name[120];
    MINODE *mip, *pmip;

    ino = getino(pathname);   //get inode#
    mip = iget(dev, ino);     //get MINODE by inode#

    //mip is a DIR, not BUSY, and EMPTY  
    if(dir_or_file(mip) == 1 && mip->refCount <= 2){
        if(child_count(mip) == 2){

            free_INODE_BLOCK(mip); //deallocate MINODE

            //get parent inode#, parent INODE, and mip name
            get_myino(mip,&pino);
            pmip = iget(mip->dev, pino);
            get_myname(pmip, ino, my_name);   
            
            rm_child(pmip, my_name);     //remove child DIR from parent DIR
            pmip->INODE.i_links_count--; 
            pmip->dirty = 1;

            //deallocate mip (CHILD DIR)
            bdalloc(mip->dev,mip->INODE.i_block[0]);
            idalloc(mip->dev,mip->ino);
            iput(mip);
            iput(pmip);
            
        }
        else{
            iput(mip);
            printf("_err: DIR NOT EMPTY\n");
            return -1;
        }
    }
    else{
        iput(mip);
        printf("_err: DIR is BUSY ; or NOT A DIR\n");
        return -1;
    }
}