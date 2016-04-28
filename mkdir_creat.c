#include "include.h"
/* a/b/c */

DIR *last_entry(int blk, int print);

int InodeHasDirectory(INODE *cur_inode, char *path){
	if (!cur_inode) return 0;	
	char buf[1024];
	char *s = "";
	while (s = (char*)parse_pathname(path, "/")){
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
	
	if (dir == 0){
		printf("making a file\n"); 
	}else{
		printf("making a directory\n");
	}
	
	MINODE *mip = root; // 1. pahtname = "/a/b/c" start mip = root;         dev = root->dev;

	if (*pathname != '/'){ mip = running->cwd; } // if pathname =  "a/b/c" start mip = running->cwd; dev = running->cwd->dev;

	dev = mip->dev;

	char temp_pathname[256];
	strcpy(temp_pathname, pathname);
	char *parent = dirname(pathname); //2. Let parent = dirname(pathname);   parent= "/a/b" OR "a/b"
	char *child  = basename(temp_pathname); //child  = basename(pathname);  child = "c"
	
	printf("parent = %s child = %s\n", parent, child);

	int pino = my_getino(&dev, parent); //3. Get the In_MEMORY minode of parent:
	MINODE *pip = iget(dev, pino);

	if (S_ISREG(pip->INODE.i_mode)){ printf("pip is not a dir!\n"); return; } //Verify : (1). parent INODE is a DIR (HOW?)
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
	int is_link = (strcmp(command_name, "symlink") == 0);
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
	//drwxr-xr-x
	ip->i_mode 	  = 0x41ED; 
	if (dir == 0) { ip->i_mode = 0x81A4; }
	if (is_link == 1) { ip->i_mode = 0xA000; }
	printf("i_mode set to %x\n", ip->i_mode);
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
	return 4 * ((8 + n + 3) / 4);
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
			printf("\n\nNO SPACE IN EXISTING DATA BLOCKS\n\n");
			getchar();
		}
		
	}	
	
	

	//printf("could not link():\n");
}

int my_rmdir(char *pathname){
	remove_item(pathname, 1);
}

int is_dir_empty(MINODE *mip)
{
	int not_empty;
	DIR *last = (DIR*)print_dir_entries(mip, 1);
	
	//printf("last = %x\n", last);

	if (last != 0) 
	{
		char last_name[6];
		strncpy(last_name, last->name, last->name_len);
		last_name[last->name_len] = 0;
		if (strcmp(last_name, "..") != 0 && strcmp(last_name, "lost+found") != 0)
		{
			printf("not empty, last name is %s\n", last_name);
			not_empty = 1;
		}
	}
	else{
		not_empty = 0;
	}
	
	return not_empty;
}

int remove_item(char *pathname, int dir){
	char backup_pathname[256];
	strcpy(backup_pathname, pathname);
	if(strcmp(pathname, ".") == 0 || strcmp(pathname, "..") == 0){
		printf("You cannot delete this folder KC, nice try!\n");
		return 0;
	}
	 
	getchar();
	char ptn_copy[256];
	strcpy(ptn_copy, pathname);
	printf("ptn_copy = %s\n", ptn_copy);
	int ino  = my_getino(&dev, pathname); //2. get inumber of pathname: determine dev, then
	if (ino == 0){
		printf("%s does not exist\n", pathname);
		return;
	}
	printf("got ino %d for pathname %s in my_rmdir()\n", ino, pathname);
	MINODE *mip = iget(dev, ino); //3. get its minode[ ] pointer:
	//4. check ownership super user : OK not super user: uid must match 
	int super_user = 1; //TODO check this later
	int same_uids = 1; //TODO check this later
	if (super_user == 1 || same_uids == 1){}
	int dir_type = mip->INODE.i_mode == 0x41ED; //5. check DIR type (HOW?) AND not BUSY (HOW?) AND is empty:
	int busy = 0;
	
	//TODO go through its data block(s) to see whether it has any entries in addition to . and .
	
	int not_empty = (mip->INODE.i_links_count > 2) && is_dir_empty(mip);
	
	/*
	if (!dir_type || busy || not_empty){
		printf("invalid, returning\n");
		printf("dir_type = %d , imode = %x BUSY = %d not_empty = %d\n", dir_type, mip->INODE.i_mode, busy, not_empty);
		iput(mip); 
		return -1;
	}*/
	
	//6. ASSUME passed the above checks. Deallocate its block and inode
	
	int i;
	for (i = 0; i < 12; i++){
		if (mip->INODE.i_block[i]==0)
			continue;
		bdealloc(mip->dev, mip->INODE.i_block[i]);
	}
	idealloc(mip->dev, mip->ino);
	iput(mip); //(which clears mip->refCount = 0);

	getchar();
	char *parent = dirname(backup_pathname); 
	
	printf("parent in remove dir is %s\n", parent);
        int pino = my_getino(&dev, parent); 
        MINODE *pip = iget(mip->dev, pino); //	7. get parent DIR's ino and Minode (pointed by pip);

	printf("getting basename of %s\n", ptn_copy);

	char *child = basename(ptn_copy);
	
	printf("child in remove_dir is %s\n", child); 
	
	rm_child(pip, child);

	printf("got to line 265\n");

	pip->INODE.i_links_count--; //decrement pip's link_count by 1;
	pip->INODE.i_atime = pip->INODE.i_ctime = time(0L); //touch pip's atime, mtime fields;
	pip->dirty 	  = 1;
	iput(pip);
	return 1;
}

int rm_child(MINODE *parent, char *name){

	printf("parent name is %s\n", parent->name);	
	printf("name in rm_child is %s\n", name);

	char buf[1024];	

	//1. Search parent INODE's data block(s) for the entry of name
	
	INODE *inodePtr = &parent->INODE;

	//get the data block of inodePtr
	int k, i;
	char *cp;  char temp[256];
       	DIR  *dp, *last = dp, *previous;
	
       	// ASSUME INODE *ip -> INODE
	char found = 0;
	int stepped = 0;
	int blk, bts = 0;
	for (i = 0; i < 12; i++)
	{	
		if (found == 1) break;
	       	//printf("i_block[%d] = %d\n", i, inodePtr->i_block[i]); // print blk number
		blk = inodePtr->i_block[i];
	       	get_block(fd, blk, buf);     // read INODE's i_block[0]
	       	cp = buf;  
	       	dp = (DIR*)buf;
	       	while(cp < buf + BLKSIZE){
			stepped++;
			printf("Searching for %s\n", name);
	      		strncpy(temp, dp->name, dp->name_len);
	      		temp[dp->name_len] = 0;
			if (strcmp(name, temp) == 0)
			{
				printf("Found %s\n", temp); 
				found = 1;	
				break;			
			}else if (dp->rec_len == 0) { break; }
			previous = dp;
	      		cp += (dp->rec_len);   // advance cp by rec_len BYTEs
			bts += dp->rec_len;
	      		dp = (DIR*)cp;     // pull dp along to the next record
	       	}
	}

	//last, middle, first
	
	if (dp->rec_len == 0){
		printf("could not find %s\n", name);
		return;
	}
	
	if (stepped > 1 && cp + (dp->rec_len) < buf + BLKSIZE){
		show_dir(blk);

		printf("not the first one\n");
		
		strncpy(temp, dp->name, dp->rec_len);
		temp[dp->rec_len] = 0;
		printf("to delete: %s rec_len:%d name_len:%d\n", temp, dp->rec_len, dp->name_len);

		int deleted_len = dp->rec_len;
		cp = (char*)dp;
		cp += dp->rec_len;
		DIR * next = (DIR*)cp;
		
		//printf("next has address %x\n", next);

		strncpy(temp, next->name, next->name_len);
		temp[next->rec_len] = 0;
		printf("next is %s with rec_len:%d and name_len:%d\n", temp, next->rec_len, next->name_len);

		printf("finding the last one\n");
		last = next;

		while(cp < buf + BLKSIZE){
			last = (DIR*)cp;
			cp += last->rec_len;
			DIR *the_next = (DIR*)cp;
			if (the_next->rec_len == 0) break;
		}

		//print the name of the last entry
		strncpy(temp, last->name, last->name_len);
		temp[last->name_len] = 0;
		printf("last entry = %s\n", temp);
	
		int bytes = 1024 - bts;
		last->rec_len += deleted_len;
		printf("copying bytes = %d\n", bytes);
		memcpy(dp, next, bytes);

		printf("writing to block %d on fd %d\n", blk, fd);
		put_block(fd, blk, buf);

		show_dir(blk);
	}else{
		//this is when the dir entry to delete 
		//is the last one
		
		//print the previous 
		
		printf("the item to delete is the last one\n");
		
		strncpy(temp, previous->name, previous->name_len);
		temp[previous->name_len] = 0;
		printf("previous is %s\n", temp); 
		
		//print the current
		
		strncpy(temp, dp->name, dp->name_len);
		temp[dp->name_len] = 0;
		printf("to_delete is %s\n", temp);
		
		
		int deleted_len = dp->rec_len;
		int bytes = 1024 - bts;
		previous->rec_len += deleted_len;
		//cp = previous;
		memcpy(dp, previous, deleted_len);
		put_block(fd, blk, buf);
		//make previous occupy all the space 
	}
}


























