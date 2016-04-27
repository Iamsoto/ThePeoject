#include "include.h"


/**
 * Essentially the makedir function except its designed to update things...
 */
/*int create_inode_from_existing(MINODE *inode_to_copy, char *name){

	//2. allocate an inode and a disk block for the new directory;
	//int ino = ialloc(dev);
	int bno = balloc(dev);

	printf("ino in kmdir is %d and bno is %d\n", inode_to_copy->ino, bno);
	getchar();

	MINODE *mip = iget(dev, ino);  3.to load the created inode into a minode[] (in order to
  	 	 wirte contents to the INODE in memory).

	if (mip == 0)
		printf("mip is null\n");

	//4. Write contents to mip->INODE to make it as a DIR.

	//5. iput(mip); which should write the new INODE out to disk.
	INODE *ip = &mip->INODE;
	INODE *copy = &inode_to_copy->INODE;

	Set all of the MINODE and INOE properties
	//drwxr-xr-x
	ip->i_mode 	  = copy->inode;
	if (dir == 0) { ip->i_mode = 0x81A4; }
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

	Create data block for new DIR containing . and .. entries
	into a buf of BLKSIZE
	write buf to disk


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

	Enter name entry into parent's directory by enter_name(pip, ino, name)

	//enter_name(pip, ino, name);

	//write data block to disk

}*/


/**
 * TODO: - Assure we are only writing to a dir(step 3)
 * 		 - Double and Indirect blocks
 */
int my_link(char *pathname, char *parameter){
	char buf[1024];
	char the_buf2[1024]= { 0 };
	int ino = getino(&dev, pathname);
	int ino_base = getino(&dev, dirname(pathname));
	if (ino == 0){ return 0; }
	if(ino_base == 0) {return 0;}

	// The Minode trying to be copied
	MINODE *mip_to_copy = iget(dev,ino);
	// The minode of the base
	MINODE *mip_base = iget(dev,ino_base);

	printf("ino for pathname is %d\n", ino);
	
	// Check to make sure file being copied is Dir
	if ((mip_to_copy->INODE.i_mode & 0xF000) == 0x4000){
		printf("CANNOT LINK TO A DIRECTORY\n");
		return 0;
	}
	
	// Check if base exists and is Dir
	if(mip_base == 0 || (mip_base->INODE.i_mode & 0xF000) != 0x4000){
		printf("Issue Creating a link in this directory\n");
		return 0;
	}

	// Check if file-to-link already exists
	char path_new_file[200] = { 0 };
	strcpy(path_new_file, dirname(pathname));
	strcat(path_new_file, "/");
	strcat(path_new_file, parameter);
	printf("The total new path = %s\n", path_new_file);
	if(getino(path_new_file) != 0){
		printf("File to link already exists \n");
	}

	// The base inode
	INODE *ibase = &(mip_base->INODE);
	// The inode to copy
	INODE *to_copy = &(mip_to_copy->INODE);
	
	char sec_buf[1024];
	int i;
	char *cp;
	DIR *dp;

	int new_length = ideal_len(strlen(basename(parameter)));
	int current_dir_length = 0;
	int remain = 0;
	
	//Traverse all i_blocks to assure there is room
	for (i = 0; i < 12; i++){
		if (ibase->i_block[i] == 0) break;
		
		get_block(fd, ibase->i_block[i], sec_buf);
		cp = sec_buf;
		dp = (DIR *)sec_buf;
		
		int blk = ibase->i_block[i];
		printf("step to LAST entry in data block %d\n", blk);

		// Traverse all the directories in the given block...
		while (cp + dp->rec_len < sec_buf + BLKSIZE){
			current_dir_length = ideal_len(dp->name_len);
			printf("dp->rec_len = %d current_dir_length = %d\n", dp->rec_len, current_dir_length);			
			char temp[256];
			strncpy(temp, dp->name, dp->name_len);
			temp[dp->name_len] = 0;
			printf("Traversing %s\n", temp);
			cp += dp->rec_len;
			dp = (DIR*)cp;
			remain = dp->rec_len - current_dir_length;

		}// We are at the end of the block
		

		// Now to write the new node
		if (remain >= new_length){
			INODE * updated_inode = 0;
			printf("remain = %d current_dir_length = %d\n", remain, current_dir_length);
			printf("hit case where remain >= needed_length\n");	
	
			/* Enter the new entry as the last entry and trim the previous entry to its ideal length*/
			int ideal = ideal_len(dp->name_len);
			dp->rec_len = ideal;
			cp += dp->rec_len;
			dp = (DIR*)cp;

			// Obtain the inode to copy, update it, and write it back to its appropriate blk
			int cur_blk    = (mip_to_copy->ino - 1) / 8 + bg_inode_table;
			int cur_offset = (mip_to_copy->ino - 1) % 8;
			printf("Setting a block in blk: %d", cur_blk);
			get_block( dev, cur_blk, the_buf2);
			updated_inode = (INODE *)the_buf2 + cur_offset;
			updated_inode->i_links_count ++;
			put_block(fd, cur_blk, the_buf2);

			// Give 'dp' the inode we just obtained
			dp->inode    = mip_to_copy->ino;
			dp->rec_len  = remain;
			dp->name_len = strlen(basename(parameter));
			strncpy(dp->name, basename(parameter), dp->name_len);

			// Update the base node's i_block[i] To correctly show the dp we just obtained
			put_block(fd, ibase->i_block[i], sec_buf);

			// Lastly, put away those pesky minodes
			iput(mip_base);
			iput(mip_to_copy);
			return 0;

		}else{
			printf("\n\nNO SPACE IN CURRENT DATA BLOCK\n\n");
			getchar();
		}
	}
	
	iput(mip_base);
	iput(mip_to_copy);
	return 0;
	
}



















