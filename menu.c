/************* mv.c file **************/

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
 * Display a menu of all commands that a user may enter
 */
void menu(){
    printf("---------------------------MENU-----------------------------\n");
    printf("[ls|cd|pwd|mkdir|rmdir|creat|link|unlink|symlink|readlink|\n");
    printf(" stat|chmod|open|close|read|cat|pfd|cp|mv|quit]\n");
    printf("------------------------------------------------------------\n");
}