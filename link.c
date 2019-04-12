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
int link_file(){
    int old_ino;
    char old_dirname[256] ={""},old_basename[120] = {""};  //old file dirname & basename
    char new_dirname[256]={""},new_basename[120]={""};  //new file dirname & basename
    MINODE *old_mip;

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

            }
            else{
                //basename of new file already exists
                printf("_err: %s ; linker file ALREADY EXISTS", name[n-1]);
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