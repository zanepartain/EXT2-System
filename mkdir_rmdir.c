/************* mkdir_rmdir.c file **************/
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

/**
 * Divides the path into dirname and basename. Returns basename
 */
int dirname_basename(char *dirname, char *basename){

    if(n > 0){
        //set basename as last path name
        strcat(basename,name[n-1]);
    }

    if(pathname[0] == '/'){
        //exact path
        dirname[0] = '/';
    }
    
    for(int i = 0; i < n - 1; i++){
        //append path name to dirname
        strcat(dirname,name[i]);
        strcat(dirname,"/");
    }

    return basename;
}

int make_directory(){
    char dirname[120] = {""}, basename[120] = {""};
    int pino;
    MINODE *pmip;

    tokenize(pathname);  //tokenize pathname

    dirname_basename(dirname,basename);  //get basname and dirname

    pino = getino(dirname);  //get parent inode#
    pmip = iget(dev,pino);   //get parent MINODE

    //parent MINODE is a DIR
    if(dir_or_file(pmip) == 1){
        if(search(pmip,basename) == 0){
            //basename does not exist
            printf("basename = %s ; DOES NOT EXIST\t",basename);
            printf("dirname = %s \n",dirname);
        }
        else{
            //basename already exists
            printf("basename = %s ; ALREADY EXISTS\t",basename);
            printf("dirname = %s \n",dirname);
        }
    }

    
}
