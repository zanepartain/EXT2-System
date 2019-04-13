/************* cd_ls_pwd.c file **************/

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

/*IF YOU ALTER THE INODE YOU MUST WRITE BACK*/
//iput(mip)...
//mip->INODE will modify the INODE


/*
CD: Change CWD to new DIR (if DIR exists)
*/
int change_dir()
{
  int ino = getino(pathname);       //get INODE num at pathname
  MINODE *mip = iget(dev, ino);     //get MINODE that matches dev, and ino
  
  if(dir_or_file(mip) == 1){
    iput(running->cwd);  //iput the old DIR minode
    running->cwd = mip;  //set new running DIR minode
  }
  else{
    printf("Not a directory\n");
  }
}


/*
list (print) the stats of the file passed in by INODE number and FILE name
*/
int ls_file(int ino, char *fname){
  struct stat fstat, *sp;
  int r, i;
  char ftime[64];
  MINODE *mip;
  char *t1 = "xwrxwrxwr-------";
  char *t2 = "----------------";

  sp = &fstat;
  mip = iget(dev,ino);  //get INODE spec. by ino#

  INODE *ip = &(mip->INODE);  //access mip->INODE

  int type = dir_or_file(mip);  //get type of INODE

  switch (type)
  {
    case 0:
      printf("%c",'-');
      break;
    case 1:
      printf("%c",'d');
      break;
    case 2:
      printf("%c",'l');
      break;
  
    default: 
      printf("_err: unsupported FILE type");
      break;
  }

  for (i=8; i >= 0; i--)
  {
    if (ip->i_mode & (1 << i)) // print r|w|x
    {
      printf("%c", t1[i]);
    } 
    else
    {
      printf("%c", t2[i]); // or print -
    }
  }
  printf("%4d ",ip->i_links_count); // link count
  printf("%4d ",ip->i_gid);         // gid
  printf("%4d ",ip->i_uid);         // uid
  printf("%8d ",ip->i_size);        // file size
  
  strcpy(ftime, ctime(&ip->i_ctime));  // print time in calendar form
  ftime[strlen(ftime)-1] = 0;          // kill \n at end
  printf("%s ",ftime);

  printf("%s", basename(fname)); // print file basename

 
  /*        NOT WORKING
  if ((ip->i_mode & 0xF000)== 0xA000){   // print -> linkname if symbolic file
    // use readlink() to read linkname
    printf(" -> %s", linkname); // print linked name
  }
  */
  iput(mip);
  printf("\n");
}


/*
List (print) the stats of every FILE or DIR in the passed 
in DataBlock (MINODE)
*/
int ls_dir(MINODE *mip){
  char sbuf[BLKSIZE], temp[256];
  char *cp;
  get_block(dev,mip->INODE.i_block[0], sbuf);  //get dir entry

  dp = (DIR *)sbuf;
  cp = sbuf;

  while(cp < sbuf + 1024){

    if(strcmp(dp->name,".") != 0 && strcmp(dp->name,"..") != 0)
    {
      strncpy(temp, dp->name, dp->name_len);  //make name a string and save into temp
      temp[dp->name_len] = 0;                 //ensure null at end

      ls_file(dp->inode, temp);
    } 

    cp += dp->rec_len;   //advance cp by rec_len
    dp = (DIR *)cp;      //pull dp to next entry
  }
}


/*
This func. is called when user enters 'ls (pathname)'
*/
int list_file()
{
  int ino,pino;
  printf("list_file(): \n");

  //set ino to running CWD inode#
  //or specified pathname inode#
  if(strlen(pathname) == 0)
  {
    ino = get_myino(running->cwd,&pino);
  }
  else
  {
    ino = getino(pathname);
  }

  MINODE *mip = iget(dev, ino);
  if(dir_or_file(mip) == 1){       //is DIRectory
    ls_dir(mip);
  }
  else if(dir_or_file(mip) == 0){  //is FILE
    //ls_file(mip);
  }
  iput(mip);
}


/*
Returns inode# of MINODE mip, and the inode# of its parent
*/
int get_myino(MINODE *mip, int *parent_ino){
  char sbuf[BLKSIZE];
  int ino;
  char *cp;
  MINODE *pip;  //MINODE parent ptr

  get_block(dev,mip->INODE.i_block[0], sbuf);  //get first DataBlock (dir-entry) of mip
  dp = (DIR *)sbuf;    
  cp = sbuf;

  while(cp < sbuf + 1024){
    if(strcmp(dp->name,".") == 0) 
    {
      ino = dp->inode;  //mip's own inode#
    } 
    if(strcmp(dp->name,"..") == 0)
    {
      (*parent_ino) = dp->inode; //mip's parent inode#
    }

    cp += dp->rec_len;   //advance cp by rec_len
    dp = (DIR *)cp;      //pull dp to next entry
  }
  
  return ino;  //return mip ino#
}


/*
 Stores the name of the FILE with inode# == my_ino (param) into myname (param), 
 by searching the FILE's parent MINODE.
*/
int get_myname(MINODE *parent_mip, int my_ino, char *myname){
  char sbuf[BLKSIZE];
  int ino;
  char *cp;
  MINODE *mip = parent_mip;  //temp parent_mip

  get_block(dev,mip->INODE.i_block[0], sbuf);  //get first DataBlock (dir entry)
  dp = (DIR *)sbuf;    
  cp = sbuf;

    while(cp < sbuf + 1024){
      if(dp->inode == my_ino){
        strncpy(myname, dp->name, dp->name_len);  //make name a string
        myname[dp->name_len] = 0;                 //ensure null at end
      }
      cp += dp->rec_len;   //advance cp by rec_len
      dp = (DIR *)cp;      //pull dp to next entry
  }

}


int rpwd(MINODE *wd){
  int parent_ino, my_ino;
  char my_name[120];
  MINODE *pip;
 
  if (wd == root){
    return;
  }

  my_ino = get_myino(wd,&parent_ino); //get wd ino# and parent ino#
  pip = iget(dev, parent_ino);        //get parent MINODE

  get_myname(pip,my_ino,&my_name);    //get wd file name under parent MINODE

  rpwd(pip);                          //find parent of MINODE parent ptr

  printf("/%s", my_name);
}


int pwd(MINODE *wd)
{
  printf("pwd(): ");
  if (wd == root){
    printf("/\n");
  }
  else{
    rpwd(wd);
    putchar('\n');
  }
}