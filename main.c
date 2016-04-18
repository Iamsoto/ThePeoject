#include "include.h"   // include ALL YOUR .c files here

/************************ globals *****************************/

extern int cd(char *param);
extern int pwd(char *param);

char *command_name;
int dev;
static char *name[128];
char pathname[128], parameter[128], cwdname[128];
char names[128][256];

int  nnames;
char *rootdev = "disk", *slash = "/", *dot = ".";
int iblock;

MINODE *root; 
MINODE minode[NMINODES];
MOUNT  mounttab[NMOUNT];
PROC   proc[NPROC], *running;
OFT    oft[NOFT];

MOUNT *getmountp();

int DEBUG=0;
int nproc=0;

int fd = 0;
u32 bg_inode_table;

char buf[BLOCK_SIZE];

MOUNT *mp;

int bmap, imap, inode_start;
int ninodes, nblocks, ifree, bfree;

extern int mkdir_creat(char *pathname);

int put_block(int fd, int blk, char buf[ ])
{
	lseek(fd, (long)blk*BLKSIZE, 0);
	write(fd, buf, BLKSIZE);
}

int get_block(int fd, int blk, char buf[ ])
{
	lseek(fd, (long)blk*BLKSIZE, 0);
	read(fd, buf, BLKSIZE);
}


void mountroot()   /* mount root file system */
{
	int i, ino;

	char line[64], buf[BLOCK_SIZE], *rootdev;

	rootdev = "mydisk";
	
	printf("enter rootdev name (RETURN for %s) : ", rootdev);
	fgets(line, 64, stdin);
	line[strlen(line)-1] = 0;

	if (line[0] != 0)
		rootdev = line;

	dev = open(rootdev, O_RDWR);
	printf("dev = %d\n", dev);
	fd = dev;
	
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
		printf("super magic=%x : %s is not a valid Ext2 filesys\n", sp->s_magic, rootdev);
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

	bmap = mp->bmap = gp->bg_block_bitmap;	
	imap = mp->imap = gp->bg_inode_bitmap;
	mp->iblock = gp->bg_inode_table;

	strcpy(mp->name, rootdev);
	strcpy(mp->mount_name, "/");

	printf("bmap=%d  ",   gp->bg_block_bitmap);
	printf("imap=%d  ",   gp->bg_inode_bitmap);
	printf("iblock=%d\n", gp->bg_inode_table);  

	bg_inode_table = gp->bg_inode_table;

	/***** call iget(), which inc the Minode's refCount ****/
	root = iget(dev, 2);          /* get root inode */
	strcpy(root->name, "/");
	printf("size of root = %d\n", root->INODE.i_size);
	printf("Block number of root is %d\n", root->INODE.i_block[0]);
	getchar();
	mp->mounted_inode = root;
	root->mountptr = mp;

	printf("mount : %s  mounted on / \n", rootdev);
	printf("nblocks=%d  bfree=%d   ninodes=%d  ifree=%d\n", nblocks, bfree, ninodes, ifree);
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
	//print_dir_entries(root);
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

int search_array(char *function_names[], char *s)
{
	int i = 0;
	while (function_names[i]){
		if (strcmp(function_names[i], s) == 0){
			return i;
		}
		i++;
	}
	return -1;
}

int touch(char *param){}
int my_chmod(char *param){}
int chown(char *param){}
int chgrp(char *param){}

int ls(char *path)
{
	printf("bg_inode_table = %d\n", bg_inode_table);
	//search to see if the inode is in memory or not
	//if it isn't, bring it to memory
	//in ls a/b/c bring c to memory if it isn't already
	int count;
	int i;
	
	//from root or from cwd?

	int from_root = 1;
	if (*path != '/') from_root = 0;

	MINODE *current_location = root;
	if (!from_root){
		current_location = running->cwd;
	}

	//go inside every MINODE, get the INODE, search
	//its data blocks for the next entry to be found
	//if it isn't found, print this

	INODE *current_inode = &(current_location->INODE);

	printf("start_location = %s\n", current_location->name);
	printf("start_location ino = %d\n", current_location->ino);
	printf("start_location dev = %d\n", current_location->dev);

	int blk, offset, return_ino;
	char *current_part = "";
	while (current_part = (char*)parse_pathname(path)){	
		//printf("Found current_inode = %.8x\n", current_inode);
		printf("current_part = [%s]\n", current_part);
		char other_temp[256];
		strcpy(other_temp, current_part);
		return_ino = search(current_inode, other_temp);
		if (return_ino == 0){
			printf("could not find %s\n", current_part);return;		
		}else{
			printf("Found inode %d for %s\n", return_ino, current_part);
		}

		//return_ino is valid
		blk = (return_ino - 1) / 8 + bg_inode_table;
		offset = (return_ino - 1) % 8;
		printf("block %d offset %d\n", blk, offset);
		get_block(fd, blk, buf);
		current_inode = (INODE*)buf + offset;
	}

	//we have the inode we want to print
	
	//printf("Found current_inode = %.8x\n", current_inode);

	MINODE *mip = iget(dev, return_ino);
	mip->INODE = *current_inode;
	print_dir_entries(mip);			
}

int clear(){
	int i;
	for (i = 0; i < 33; i++){
		printf("\n");
	}
}

int my_open(){
	return 0;
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
	 
	  char *function_names[] = {"touch", "chmod", "chown", "chgrp", "ls", "cd", "clear", "open", "mkdir", "creat", "pwd", 0};
	  int (*fptr[])() = {touch, my_chmod, chown, chgrp, ls, cd, clear, my_open, mkdir_creat, mkdir_creat, pwd, 0};

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
		
		fgets(line,128,stdin);
		line[strlen(line)-1]=0;
		if (line[0]==0) continue;

		sscanf(line, "%s %s %64c", cname, pathname, parameter);
		command_name = cname;
		int function_index = search_array(function_names, cname);
		if (function_index == -1){
			printf("Yo, the %s command is invalid\n", cname);
		}
		else{
			int return_val = fptr[function_index](pathname, parameter);
		}
	  }

} /* end main */

// NOTE: you MUST use a function pointer table


