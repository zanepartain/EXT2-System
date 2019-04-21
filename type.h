/*************** type.h file ************************/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;
#define BLKSIZE  1024

SUPER  *sp;
GD     *gp;
INODE  *ip;
DIR    *dp;   

char  buffer[256];
char  mode[2];

//Default dir and regular file modes
#define DIR_MODE      0x41ED
#define FILE_MODE     0x81AE
#define FILE_MODE_STD 0100644
#define LNK           0120777
#define SUPER_MAGIC   0xEF53
#define SUPER_USER    0
//PROC status
#define FREE          0
#define READY         1

//file system table sizes
#define NMINODE    64
#define NFD         8
#define NPROC       2
#define NMTABLE    10
#define NOFT       40

typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  // for level-3
  int mounted;
  struct mntable *mptr;
}MINODE;

typedef struct oft{
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;

typedef struct proc{
  struct proc *next;
  int          pid;
  int          uid;
  int          status;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;

typedef struct mtable{
  int dev;              //device number; 0 or FREE
  int ninodes;          //from superblock
  int nblocks;          
  int free_blocks;      //from superblock and GD
  int free_inodes;
  int bmap;             //from group descriptor
  int imap;
  int iblock;           //inodes start block
  MINODE *mntDirPtr;    //mount point to DIR pointer
  char devName[64];     //device name
  char mntName[64];     //mount point DIR name
}MTABLE;

MTABLE *mp;
