#include "include.h"   // include ALL YOUR .c files here

/************************ globals *****************************/
MINODE *root; 
char pathname[128], parameter[128], *name[128], cwdname[128];
char names[128][256];

int  nnames;
char *rootdev = "disk", *slash = "/", *dot = ".";
int iblock;

MINODE minode[NMINODES];
MOUNT  mounttab[NMOUNT];
PROC   proc[NPROC], *running;
OFT    oft[NOFT];

MOUNT *getmountp();

int DEBUG=0;
int nproc=0;

extern int get_block(int fd, int blk, char buf[ ]);
extern int search(INODE * inodePtr, char * name);

int put_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  write(fd, buf, BLKSIZE);
}

void cd(char *pathname)
{

	if (strlen(pathname) == 0 )
	{

	}



}

int getino(int *dev, char *pathname)
{
	int current_dev = root->dev;
	if (*pathname != '/')
		current_dev = running->cwd->dev;
			
	//tokenize pathname

	if (strlen(pathname) == 0){ //refers to root

	}

	char temp[256];
	strcpy(pathname, temp);
	
	char *s = strtok(temp, "/");
	printf("%s\n", s);
	while (s = strtok(0, "/")){
		printf("%s\n", s);
	}
}

int iput(MINODE *mip)
{
	mip->refCount--; //decrease refCount by 1
	if (mip->refCount > 0){
		return;
	}
	if (mip->dirty == 0){
		return;
	}
	if (mip->refCount > 0 && mip->dirty == 1){
		//must write the INODE back to disk
		int dev    = mip->dev;
		int ino    = mip->ino;
		int blk    = (ino - 1) / 8 + bg_inode_table;
		int offset = (ino - 1) % 8;
		get_block(fd, blk, buf);
		INODE *ip = (INODE *)buf + offset;
		*ip = mip->INODE;
		put_block(fd, blk, buf);
	}
}

MINODE *iget(int dev, int ino)
{
	/*
	(1). Search minode[i] for an entry whose refCount > 0 with the SAME (dev,ino)
     	     if found: refCount++; mip-return &minode[i];
	(2). Find a minode[i] whose refCount = 0 => let MINODE *mip = &minode[i];
	*/

	MINODE *mip = 0;
	int i;
	for (i = 0; i < NMINODES; i++)
	{
		if (minode[i].dev == dev && minode[i].ino == ino){
			minode[i].refCount++;
			return &minode[i];
		}else if (minode[i].refCount == 0){
			mip = &minode[i];
		}	
	}

      /*(3). Use Mailman's algorithm to compute*/

	int blk = (mip->ino - 1) / 8 + bg_inode_table;
	int offset = (mip->ino - 1) % 8;

    /*  (4). read blk into buf[ ]; 	*/

	get_block(fd, blk, buf);
	INODE *ip = (INODE *)buf + offset;
	
      //(5). COPY *ip into mip->INODE

	mip->INODE = *ip;

     // (6). initialize other fields of *mip: 

	mip->ino      = dev; 
	mip->dev      = ino;

	mip->refCount = 1;
	mip->dirty    = 0;
	mip->mounted  = 0;
	mip->mountptr = 0;

	return 0;
}

void mountroot()   /* mount root file system */
{
	  int i, ino, fd, dev;
	  MOUNT *mp;
	  SUPER *sp;
	  MINODE *ip;

	  char line[64], buf[BLOCK_SIZE], *rootdev;
	  int ninodes, nblocks, ifree, bfree;

	  printf("enter rootdev name (RETURN for disk) : ");
	  gets(line);

	  rootdev = "disk";

	  if (line[0] != 0)
	     rootdev = line;

	  dev = open(rootdev, O_RDWR);
	  if (dev < 0){
	     printf("panic : can't open root device\n");
	     exit(1);
	  }

	  /* get super block of rootdev */
	  get_block(dev, 1, buf);
	  sp = (SUPER *)buf;

	  /* check magic number */
	  printf("SUPER magic=0x%x  ", sp->s_magic);
	  if (sp->s_magic != SUPER_MAGIC){
	     printf("super magic=%x : %s is not a valid Ext2 filesys\n",
		     sp->s_magic, rootdev);
	     exit(0);
	  }

	  mp = &mounttab[0];      /* use mounttab[0] */

	  /* copy super block info to mounttab[0] */
	  ninodes = mp->ninodes = sp->s_inodes_count;
	  nblocks = mp->nblocks = sp->s_blocks_count;
	  
	  bfree = sp->s_free_blocks_count;
	  ifree = sp->s_free_inodes_count;

	  get_block(dev, 2, buf);
	  gp = (GD *)buf;

	  mp->dev = dev;         
	  mp->busy = BUSY;

	  mp->bmap = gp->bg_block_bitmap;
	  mp->imap = gp->bg_inode_bitmap;
	  mp->iblock = gp->bg_inode_table;

	  strcpy(mp->name, rootdev);
	  strcpy(mp->mount_name, "/");


	  printf("bmap=%d  ",   gp->bg_block_bitmap);
	  printf("imap=%d  ",   gp->bg_inode_bitmap);
	  printf("iblock=%d\n", gp->bg_inode_table);  


	  /***** call iget(), which inc the Minode's refCount ****/

	  root = iget(dev, 2);          /* get root inode */
	  mp->mounted_inode = root;
	  root->mountptr = mp;

	  printf("mount : %s  mounted on / \n", rootdev);
	  printf("nblocks=%d  bfree=%d   ninodes=%d  ifree=%d\n",
		  nblocks, bfree, ninodes, ifree);

	  return(0);
} 

void init()
{
	  int i, j;
	  PROC *p;

	  for (i=0; i<NMINODES; i++)
	      minode[i].refCount = 0;

	  for (i=0; i<NMOUNT; i++)
	      mounttab[i].busy = 0;

	  for (i=0; i<NPROC; i++){
	      proc[i].status = FREE;
	      for (j=0; j<NFD; j++)
		  proc[i].fd[j] = 0;
	      proc[i].next = &proc[i+1];
	  }

	  for (i=0; i<NOFT; i++)
	      oft[i].refCount = 0;

	  printf("mounting root\n");
	    mountroot();
	  printf("mounted root\n");

	  printf("creating P0, P1\n");
	  p = running = &proc[0];
	  p->status = BUSY;
	  p->uid = 0; 
	  p->pid = p->ppid = p->gid = 0;
	  p->parent = p->sibling = p;
	  p->child = 0;
	  p->cwd = root;
	  p->cwd->refCount++;

	  p = &proc[1];
	  p->next = &proc[0];
	  p->status = BUSY;
	  p->uid = 2; 
	  p->pid = 1;
	  p->ppid = p->gid = 0;
	  p->cwd = root;
	  p->cwd->refCount++;
	  
	  nproc = 2;
}

int quit()
{
	  // write YOUR quit function here
	   exit(0);
} 

int main(int argc, char *argv[ ]) 
{
	  int i,cmd; 
	  char line[128], cname[64];

	  if (argc>1){
	    if (strcmp(argv[1], "-d")==0)
		DEBUG = 1;
	  }

	  init();

	  while(1){
	      printf("P%d running: ", running->pid);

	      /* zero out pathname, parameter */
	      for (i=0; i<64; i++){
		  pathname[i]=parameter[i] = 0;
	      }      
	      /* these do the same; set the strings to 0 */
	      memset(pathname, 0, 64);
	      memset(parameter,0, 64);

	      printf("input command : ");
	      gets(line);
	      if (line[0]==0) continue;

	      sscanf(line, "%s %s %64c", cname, pathname, parameter);
	  }

} /* end main */

// NOTE: you MUST use a function pointer table





