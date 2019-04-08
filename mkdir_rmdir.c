/************* mkdir_rmdir.c file **************/

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
 * Test and Set bits in buf 
 */
int test_bit(char *buf, int bit){
    return buf[bit/8] & (1 << (bit % 8));
}
int set_bit(char *buf, int bit){
    buf[bit/8] |= (1 << (bit % 8));
}


/**
* Decrement or increment the free blocks count in both the
* SUPER block and GROUP_DESCRIPTOR block.
*/
int dec_inc_freeINODES(int dev, char action){
    char   buf[BLKSIZE];
    SUPER *super;
    GD    *group_descriptor;

    get_block(dev,1,buf);  //get super block
    super = (SUPER *)buf;

    switch (action)
    {
        case '+':
            //put super block
            super->s_free_blocks_count++;
            put_block(dev,1,buf);
            
            //get and put group descriptor block
            get_block(dev,2,buf);  
            group_descriptor = (GD *)buf;
            group_descriptor->bg_free_blocks_count++;
            put_block(dev,2,buf);
            break;

        case '-':
            //put super block
            super->s_free_blocks_count--;
            put_block(dev,1,buf);
            
            //get and put group descriptor block
            get_block(dev,2,buf);  
            group_descriptor = (GD *)buf;
            group_descriptor->bg_free_blocks_count--;
            put_block(dev,2,buf);
            break;

        default:
            printf("_err: Invalid action ; dec_inc_freeINODES");
            break;
    }
}


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


/**
* Makes a new child DIR inside of the specified parent DIR.
* (iff parent DIR exists)
*/
int make_directory(){
    char dirname[120] = {""}, basename[120] = {""};
    int pino;
    MINODE *pmip;

    tokenize(pathname);  //tokenize pathname

    dirname_basename(dirname,basename);  //get basname and dirname

    /** get parent inode# */
    if(strlen(dirname) == 0)
    {
        //get cwd inode#
        get_myino(running->cwd,&pino); 
    }
    else
    {
        //get parent inode# from dirname 
        pino = getino(dirname);        
    }

    pmip = iget(dev,pino);  //get parent MINODE

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

