#include "include.h"

int link(){
	char buf[1024];
	//(1). get the INODE of /a/b/c into memory: mip->minode[ ]
        //                                       INODE of /a/b/c
        //                                      dev,ino
        //                                       .......

	int ino = getino(&dev, pathname);
	
	if (ino == 0) return;
	printf("ino for pathname is %d\n", ino);
	
	//(2). check /a/b/c is a REG or LNK file (link to DIR is NOT allowed). 
	int ptn_blk    = (ino - 1) / 8 + bg_inode_table;
	int ptn_offset = (ino - 1) % 8;
	get_block(dev, ptn_blk, buf);
	INODE *ptn_ino = (INODE*)buf + ptn_offset;
	
	if (S_ISDIR(ptn_ino->i_mode)){
		printf("CANNOT LINK TO A DIRECTORY\n");
		return;
	}
	
	//(3). check /x/y  exists and is a DIR but 'z' does not yet exist in /x/y/
	
	char pm_cpy[256];
	strcpy(pm_cpy, parameter);
	int param_ino = getino(&dev, dirname(parameter));
	int param_blk    = (param_ino - 1) / 8 + bg_inode_table;
	int param_offset = (param_ino - 1) % 8;
	get_block(dev, param_blk, buf);
	INODE *current = (INODE*)buf + param_offset;
	int param_child_ino = search(current, basename(pm_cpy));
	if (param_ino == 0 || param_child_ino != 0){
		printf("Invalid paths\n");
		return;
	}
	
	//(4). Add an entry [ino rec_len name_len z] to the data block of /x/y/
	//This creates /x/y/z, which has the SAME ino as that of /a/b/c
	
	char sec_buf[1024];
	int i;
	char *cp;
	DIR *dp;
	int new_length         = ideal_len(strlen(basename(parameter)));
	int current_dir_length = 0;
	int remain 	       = 0;
	
	for (i = 0; i < 12; i++){
		if (current->i_block[i] == 0) break; 
		get_block(fd, current->i_block[i], sec_buf);
		
		dp = (DIR *)sec_buf;
		cp = sec_buf;
		
		int blk = current->i_block[i];
		printf("step to LAST entry in data block %d\n", blk);
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
		}
		
		if (remain >= new_length){
			
			printf("remain = %d current_dir_length = %d\n", remain, current_dir_length);
			printf("hit case where remain >= needed_length\n");	
	
			/*Enter the new entry as the last entry and trim the previous entry to its ideal length*/
			int ideal = ideal_len(dp->name_len);
			dp->rec_len = ideal;

			cp += dp->rec_len;
			dp = (DIR*)cp;
			
			dp->inode    = ino;
			dp->rec_len  = remain;
			dp->name_len = strlen(basename(parameter));
			strncpy(dp->name, basename(parameter), dp->name_len);

			put_block(fd, current->i_block[i], sec_buf);
			current->i_links_count++;

			return;
		}else{
			printf("\n\nNO SPACE IN EXISTING DATA BLOCKS\n\n");
			getchar();
		}
	}
	
	ptn_ino->i_links_count++;
	get_block(dev, ptn_blk, buf);
	put_block(dev, ptn_blk, buf);
	
	return 0;
	
}



















