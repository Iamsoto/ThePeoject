#include "include.h"
/* a/b/c */

int InodeHasDirectory(INODE *cur_inode, char *path){
	if (!cur_inode) return 0;	
	char buf[1024];
	char *s;
	while (s = parse_pathname(path, "/")){
		int cur_ino = search(cur_inode, s);
		if (cur_ino == 0){
			return 0;
		} 
		int cur_blk = (cur_ino - 1) / 8 + bg_inode_table;
		int offset = (cur_ino - 1) % 8;
		get_block(fd, cur_blk, buf);
		cur_inode = (INODE*)buf + offset; 
	}
	return 1;
}

int mkdir_creat(char *pathname){
	int dir  = (strcmp(command_name,"mkdir") == 0);
	MINODE *mip = root; // 1. pahtname = "/a/b/c" start mip = root;         dev = root->dev;

	if (*pathname != '/'){ mip = running->cwd; } // if pathname =  "a/b/c" start mip = running->cwd; dev = running->cwd->dev;

	dev = mip->dev;

	char *parent = dirname(pathname); //2. Let parent = dirname(pathname);   parent= "/a/b" OR "a/b"
	char *child  = basename(pathname); //child  = basename(pathname);  child = "c"

	int pino = getino(&dev, parent); //3. Get the In_MEMORY minode of parent:
	MINODE *pip = iget(dev, pino);

	if (pip->INODE.i_mode != 0x41ED){ printf("pip is not a dir!\n"); return; } //Verify : (1). parent INODE is a DIR (HOW?)
	//(2). child does NOT exists in the parent directory (HOW?);
	if (search(&pip->INODE, child)){ printf("Child already exists in directory\n"); return; } 

	printf("child is %s\n", child);

	mymkdir(pip, child, pino, dir);
	if (dir) pip->INODE.i_links_count++; // inc parent inodes's link count by 1; 
	else pip->INODE.i_links_count = 0;
	
	pip->dirty = 1; // mark it DIRTY
	pip->INODE.i_atime = time(0L); //touch its atime
	iput(pip);
}


int mymkdir(MINODE *pip, char *name, int pino, int dir){
	//2. allocate an inode and a disk block for the new directory;
	int ino = ialloc(dev);    
	int bno = balloc(dev);

	printf("ino in kmdir is %d and bno is %d\n", ino, bno);
	getchar();

	MINODE *mip = iget(dev, ino); /* 3.to load the inode into a minode[] (in order to
  	 wirte contents to the INODE in memory). */
	
	if (mip == 0)
		printf("mip is null\n");

	//4. Write contents to mip->INODE to make it as a DIR.	

	//5. iput(mip); which should write the new INODE out to disk.
	INODE *ip = &mip->INODE;

	/*Set all of the MINODE and INOE properties*/
	ip->i_mode 	  = 0x41ED; 
	if (dir == 0) { ip->i_mode = 0x81A4; }
	ip->i_uid    	  = running->uid;
	ip->i_gid  	  = running->gid;
	ip->i_size	  = 1024;
	if (dir == 0) { ip->i_size = 0; }
	ip->i_links_count = 2; 
	if (dir == 0) { ip->i_links_count = 1; }
	ip->i_atime       = ip->i_ctime = ip->i_mtime = time(0L);
	ip->i_blocks      = 2;
	ip->i_block[0]    = bno;
	
	int i = 1;
	if (dir == 0) { i = 0; }
	for (i = 1; i < 15; i++){
		ip->i_block[i] = 0;
	}

	mip->dirty        = 1;
	iput(mip);

	/*Create data block for new DIR containing . and .. entries
	into a buf of BLKSIZE 
	write buf to disk
	*/

	char temp_buf[1024] = { 0 };
	char *cp = temp_buf;
	
	DIR *dp = (DIR*)temp_buf;

	dp->inode    = ino;
	dp->rec_len  = 12;
	dp->name_len = 1;
	strcpy(dp->name, ".");
		
	cp += dp->rec_len;
	dp = (DIR*)cp;

	dp->inode    = pino;
	dp->rec_len  = 1024 - 12;
	dp->name_len = 2;
	strcpy(dp->name, ".."); 

	put_block(dev, bno, temp_buf);

	/*Enter name entry into parent's directory by enter_name(pip, ino, name)*/

	enter_name(pip, ino, name);
	
	//write data block to disk
	
}

int ideal_len(int n){
	return (8 + n + 3);
}

int enter_name(MINODE *pip, int myino, char *myname){

	char buf[1024];

	printf("fd = %d dev = %d\n", fd, pip->dev);

	INODE current_inode = pip->INODE;	

	char *cp;
	DIR *dp;
	int i;
	int new_length = ideal_len(strlen(myname));
	int current_dir_length = 0;
	int remain = 0;

	for (i = 0; i < 12; i++){
		if (current_inode.i_block[i] == 0) break;
		//get the parent's ith data block into a buf

		//Step to the last entry in a data block (HOW?).
	
		get_block(pip->dev, pip->INODE.i_block[i], buf);

		dp = (DIR *)buf;
		cp = buf;

		int blk = pip->INODE.i_block[i];
		printf("step to LAST entry in data block %d\n", blk);
		while (cp + dp->rec_len < buf + BLKSIZE){
			current_dir_length = ideal_len(dp->name_len);
			printf("dp->rec_len = %d current_dir_length = %d\n", dp->rec_len, current_dir_length);			
			char temp[256];
			strncpy(temp, dp->name, dp->name_len);
			temp[dp->name_len] = 0;
			printf("Traversing %s\n", temp);
			cp += dp->rec_len;
			dp = (DIR*)cp;
			remain = dp->rec_len - current_dir_length;
		}
		current_dir_length = ideal_len(dp->name_len);
		remain = dp->rec_len - current_dir_length;
		if (remain >= new_length){
			
			printf("remain = %d current_dir_length = %d\n", remain, current_dir_length);
			printf("hit case where remain >= needed_length\n");	
	
			/*Enter the new entry as the last entry and trim the previous entry to its ideal length*/
			int ideal = ideal_len(dp->name_len);
			dp->rec_len = ideal;

			cp += dp->rec_len;
			dp = (DIR*)cp;

			dp->inode    = myino;
			dp->rec_len  = remain;
			dp->name_len = strlen(myname);
			strncpy(dp->name, myname, dp->name_len);

			put_block(pip->dev, current_inode.i_block[i], buf);

			return;
		}else{
			printf("NO space in existing data blocks\n");
			getchar();
		}
		
	}	
	
	

	//printf("could not link():\n");
}


/*

	1. balloc
	2. enter_name
*/

























