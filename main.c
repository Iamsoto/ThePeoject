#include "include.h"   // include ALL YOUR .c files here

/************************ globals *****************************/

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

extern int cd(char *param);
extern int pwd(char *param);
extern char * totalPath();

char *command_name;

char the_buf2[1024] = { 0 };
char completePath[1024];

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

extern int my_chgrp(char * pathname, char * group_id);
extern int mkdir_creat(char *pathname);
extern int laopen_file(char *pathname, char* str_mode );
extern int laclose_file(int fd);
extern int my_rmdir(char *pathname);
extern int my_chmod(char *pathname);
extern int chown();
extern int touch(char *pathname);
extern int my_write();
extern int laread(int fd,char buf[], int nbytes);
extern int my_cat(char *pathname);
extern int my_link(char *pathname, char *param);
extern MINODE *getParentNode(INODE * ip, int inum);
extern MINODE *getParentMinode(INODE * ip, int inum);

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

int chgrp(char *param){}

int parse(char *param){
	char *s;
	while (s = (char*)parse_pathname(param, 0)){
		printf("[%s]\n", s);
	}
}

int ls_format(char * name, INODE* inode){

	//struct stat fstat, *sp;
	int r, i;
	char ftime[64];

	//sp = &fstat;
	//printf("name=%s\n", fname); getchar();

	if ((inode->i_mode & 0xF000) == 0x8000){
		//printf(ANSI_COLOR_BLUE);
		printf("%c",'f');
	}

	if ((inode->i_mode & 0xF000) == 0x4000){

		printf("%c",'d');
	}
	if ((inode->i_mode & 0xF000) == 0xA000){
		//printf(ANSI_COLOR_RED);
		printf("%c",'l');
	}

	//printf("imode before shifting is %d\n", inode->i_mode);
	
	unsigned int permissions = (inode->i_mode << 7) >> 7;
	/*
	for (i = 15; i >= 0; i--){
		printf("%d", (permissions & (1 << i)) >> i);
	}*/
	
	//printf(" permissions are %hd ", permissions);
	
	for (i=8; i >= 0; i--){
		char *bts = "rwx";
		int cur_bt = (permissions & (1 << i)) >> i;
		//printf("%d", cur_bt);	
		if (cur_bt == 0) putchar('-');
		else{
			printf("%c", bts[(i + 1) % 3]);
		}
	}


	  printf("%4d ",inode->i_links_count);
	  printf("%4d ",inode->i_gid);
	  printf("%4d ",inode->i_uid);
	  printf("%8d ",inode->i_size);



/*	  // print time - not working
	  strcpy(ftime, inode->i_ctime);
	  ftime[strlen(ftime)-1] = 0;
	  printf("%s  ",ftime);
	  return 0;*/
	  // print name
	  printf("%s", name);


	  printf("\n");
}


/**
 * A revamped LS command...
 *
 * TODO: Double & indirect blocks
 */
 
int ls2(char *path){
	// Variables
	MINODE *mip_to_list = 0;
	char full_path[224] = { 0 };
	char the_buf[1024]= { 0 };
	char * cp= 0;
	DIR * dp;
	DIR * dp2;
	char temp_name[100] = { 0 };

	printf("************* LS 9000 ****************\n");
	printf("*************          ***************\n");

	// If search path from root
	if (path[0] == '/'){

		int ino = getino(&dev, path);
		mip_to_list = iget(dev,ino);
	}
	else { // Obtain path from current directory

		strcpy(full_path, totalPath());

		if((strcmp(path, "") !=0) && (strcmp(path, " ") !=0)){
			strcat(full_path, "/");
			strcat(full_path, path);
		}
		int ino = getino(&dev, full_path);
		mip_to_list = iget(dev,ino);
	}


	int i =0;
	for(i=0; i< 12; i++){ // Search i_blocks of mip_To_list

		if(mip_to_list->INODE.i_block[i] == 0){
			printf("End ls\n");
			return 0;
		}

		// Obtain The block that contains the dirs
		get_block(fd, mip_to_list->INODE.i_block[i], the_buf);


		cp = the_buf;
		dp = (DIR *)the_buf;
		if (dp->rec_len == 0){return 0;}

		/*
		// Skip Self
		cp += dp->rec_len;
		dp = (DIR *)cp;
		if (dp->rec_len == 0){ return 0; }
		// Skip Parent
		cp += dp->rec_len;
		dp = (DIR *)cp;
		*/

		// Traverse all directories in this block
		while (cp < the_buf + BLKSIZE){
			if (dp->rec_len == 0){

				return 0;
			}

			// Now to find the actual Inode of the directory
			int cur_blk    = (dp->inode - 1) / 8 + bg_inode_table;
			int cur_offset = (dp->inode - 1) % 8;


			char tempy[50] = { 0 };
			strncpy(tempy,dp->name, dp->name_len);
			tempy[dp->name_len] = 0;

			get_block( dev, cur_blk, the_buf2);

			INODE * dip = 0;
			dip = (INODE *)the_buf2 + cur_offset;


			strncpy(temp_name, dp->name, dp->name_len);
			temp_name[dp->name_len] = 0;

			// Format the output
			//printf(ANSI_COLOR_CYAN);
			ls_format(temp_name, dip);
			//printf(ANSI_COLOR_RESET);

			cp += dp->rec_len;
			dp = (DIR *)cp;
		}
	}

}

int clear(){
	int i;
	for (i = 0; i < 33; i++){
		printf("\n");
	}
}

int main(int argc, char *argv[ ]) 
{
	//system("color 02");
	int i,cmd; 
	char line[128], cname[64];

	if (argc>1){
		if (strcmp(argv[1], "-d")==0)
			DEBUG = 1;
	}

	init();

	char *function_names[] = {"chgrp","touch", "chmod", "chown", "chgrp", "ls", "cd", "clear", "open", "mkdir", "creat", "pwd", "rmdir", "write", "cat", "parse", "link", 0};
	int (*fptr[])() = {my_chgrp, touch, my_chmod, chown, chgrp, ls2, cd, clear, laopen_file, mkdir_creat, mkdir_creat, pwd, my_rmdir, my_write, my_cat, parse, my_link, 0};

	//test_write();

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
			printf("getting function at index %d\n", function_index);
			getchar();
			int return_val = fptr[function_index](pathname, parameter);
		}
	}

} /* end main */

// NOTE: you MUST use a function pointer table


