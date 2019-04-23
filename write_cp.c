/************* write_cp.c file **************/

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


