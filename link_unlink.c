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
 * Hard link from the new_file (sourcepath) to old_file (pathname)
 */
int link(){
    int old_ino, pino;
    char new_dirname[256]={""},new_basename[120]={""};  //new file dirname & basename
    MINODE *old_mip, *pmip;

    printf("pathname = %s\n",pathname);
    printf("sourcepath = %s\n",sourcepath);

    //get old file inode#
    old_ino = getino(pathname);    

    if(old_ino != 0)  //(if exists)
    {
        //get old file MINODE
        old_mip = iget(dev, old_ino);  

        if(dir_or_file(old_mip) != 1)  //(if not DIR)
        {
            int new_ino = getino(sourcepath); //get new_ino & tokenize sourepath

            if(new_ino == 0){
                printf("GOOD TO GO ~~ LINK BOTH FILES\n");
                dirname_basename(new_dirname,new_basename);  //get dirname & basename of sourcepath

                printf("basename = %s\t",new_basename);
                printf("dirname = %s  ; sourcepath\n",new_dirname);
                
                //get the parent DIR MINODE
                pino = getino(new_dirname);  
                pmip = iget(dev, pino);     
                //create new entry under parent, w/ old inode#
                enter_name(pmip, old_ino, new_basename); 

                old_mip->INODE.i_links_count++;  //increment old mip link count
                old_mip->dirty = 1;              //mark old mip dirty
                
                //write MINODES
                iput(old_mip);  
                iput(pmip);
            }
            else{
                //basename of new file already exists
                printf("_err: %s ; linker file ALREADY EXISTS\n", name[n-1]);
            }
        }
        else{
            //basename of old file path is DIR
            printf("_err: %s ; file_to_link is DIR\n", name[n-1]);
        }
    }
    else{
        return;
    }
}


/**
 * Unlink the specified file (if exists).
 * Delete file from parent DIR, and dec INODES link_count
 */
int unlink(){
    int ino, pino;
    char dirname[256] = {""}, basename[256] = {""};
    MINODE *mip, *pmip;

    ino = getino(pathname);  //get inode# of file

    if(ino != 0)  //(file Exists)
    {
        //get MINODE
        mip = iget(dev,ino);
        //(if MINODE is not a DIR)
        if(dir_or_file(mip) != 1)  
        {
            dirname_basename(dirname,basename); //get dirname & basename of mip

            //Get parent MINODE
            pino = getino(dirname);
            pmip = iget(dev, pino);

            //remove child file from parent MINODE
            //mark parent as dirty & write back
            rm_child(pmip,basename);
            pmip->dirty = 1; 
            iput(pmip);

            mip->INODE.i_links_count--;

            if(mip->INODE.i_links_count > 0){
                mip->dirty = 1;
            }
            else{
                //deallocate mip
                free_INODE_BLOCK(mip);
            }

            iput(mip);
        }
        else
        {
            //MINODE is a DIR
            printf("_err: %s ; file_to_unlink is DIR\n", name[n-1]);
        }
    }
    else  //(file DNE)
    {
        return;
    }
}


/**
 * Creates symbolic link from a new file to an old file
 */
int symlink(){
    int old_ino, pino, old_pino;
    char new_dirname[256]={""},new_basename[120]={""};  //new file dirname & basename
    char old_dirname[256]={""},old_basename[120]={""};  //old file dirname & basename
    char old_name[120] = {""};
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
            
            enter_name(new_mip, old_ino,old_basename); //store old file name in new file
            
            //write back new MINODE
            new_mip->dirty = 1;
            iput(new_mip);

            //get new parent MINODE & mark dirty, then write back
            pino = getino(new_dirname);
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