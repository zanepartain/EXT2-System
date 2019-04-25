/************* link.c file **************/

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
 * Creates symbolic link from a new file to an old file
 */
int symlink(){
    int old_ino, pino = 0, old_pino;
    char new_dirname[256]={""},new_basename[120]={""};  //new file dirname & basename
    char old_dirname[256]={""},old_basename[120]={""};  //old file dirname & basename
    char old_name[120] = {""};
    char sbuf[BLKSIZE];
    MINODE *old_mip, *new_mip, *new_pmip;

    //get old file inode#
    old_ino = getino(pathname);  
    //get old file MINODE dirname & basename
    dirname_basename(old_dirname,old_basename);  

    if(old_ino != 0)  //(if exists)
    {
        //get new_ino
        //get new file MINODE dirname & basename
        int new_ino = getino(sourcepath); 
        dirname_basename(new_dirname,new_basename);

        if(new_ino == 0){
            //basename of new file doesnt exist
            //create the new file
            printf("GOOD TO GO ~~ SYMLINK BOTH MINODES\n");
            creat_file(sourcepath);
            //get new created MINODE & set mode to LINK
            new_ino = getino(sourcepath);  
            new_mip = iget(dev,new_ino);   
            new_mip->INODE.i_mode = LNK;
            
            new_mip->INODE.i_size = strlen(old_basename);
            new_mip->INODE.i_block[0] = balloc(dev);
            get_block(dev, new_mip->INODE.i_block[0],sbuf);

            dp = (DIR *)sbuf;

            dp->rec_len = strlen(old_basename);
            dp->name_len = strlen(old_basename);
            dp->inode = 0;
            strcpy(dp->name, old_basename);
            
            put_block(dev, new_mip->INODE.i_block[0],sbuf);
            //write back new MINODE
            new_mip->dirty = 1;
            iput(new_mip);

            //get new parent MINODE & mark dirty, then write back
            if(strlen(new_dirname) == 0){
                pino = running->cwd->ino;
            }
            else{
                pino = getino(new_dirname);
            }
            new_pmip = iget(dev, pino);
            new_pmip->dirty = 1;

            iput(new_pmip);

        }
        else{
            //basename of new file already exists
            printf("_err: %s ; linker file ALREADY EXISTS\n", name[n-1]);
        }
    }
    else{
        return;
    }
}


/**
 * Read target file name of a symbolic
 */
int readlink(char *file, char *buffer){
    int ino, i;
    char dirname[256] = {""}, basename[256] = {""}, sbuf[BLKSIZE]; //for debug purposes
    char *cp;
    MINODE *mip;

    ino = getino(file);
    mip = iget(dev,ino);

    dirname_basename(dirname,basename);  //get dirname & basename of pathname

    //verify mip is symlink
    if(dir_or_file(mip) == 2){

        //search INODE data blocks for target name
        for(i = 0; i < 12; i ++){
            if(mip->INODE.i_block[i] == 0){
                continue;
            }
            
            //read in current block to sbuf
            //then get first dir entry
            get_block(mip->dev, mip->INODE.i_block[i], sbuf);
            dp = (DIR *)sbuf;
            cp = sbuf;

            //copy dp name to buffer
            strncpy(buffer, dp->name, dp->name_len);
            buffer[dp->name_len] = 0;
            printf("target name=%s\n",buffer);
        }
    }
    else{
        printf("_err: %s ; NOT SYMLINK\n", basename);
    }

    iput(mip);
    return mip->INODE.i_size; 
}