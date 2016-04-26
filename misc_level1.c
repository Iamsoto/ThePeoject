#include "include.h"

char nthBit(int value, int bit){
	return (value & (1 << bit)) >> bit;
}

int chown(){
	char buf[1024];
	printf("pathname = %s parameter = %s\n", pathname, parameter);
	int uid = atoi(pathname);

	if (uid == 0){
		printf("%s is not a new valid mode\n", pathname);
	}
	//are we relative to the root or relative to cwd?
	INODE *current = &root->INODE;
	if (*pathname != '/'){
		current = &running->cwd->INODE;
		//pathname++;
	}
	
	char *piece = "";
	int blk;
	while( piece = (char*)parse_pathname(parameter) ){
		printf("Current piece is %s\n", piece);
		int ino = search(current, piece);
		if (ino == 0){
			printf("Could not find %s\n", piece);
			return;
		}
		else{
			printf("Found %s with inode number %d\n", piece, ino);
		}
		blk = (ino - 1) / 8 + bg_inode_table;
		int offset = (ino - 1) % 8;
		//get the next inode
		get_block(fd, blk, buf);
		current = (INODE *)buf + offset;
	}
	
	current->i_uid = uid;
	put_block(fd, blk, buf);
}

int my_stat(char *pathname, struct stat *st){

}

int touch(char *pathname){
	char buf[1024];
	//are we relative to the root or relative to cwd?
	INODE *current = &root->INODE;
	if (*pathname != '/'){
		current = &running->cwd->INODE;
		//pathname++;
	}
	
	char *piece = "";
	int blk;
	while( piece = (char*)parse_pathname(parameter, 0) ){
		printf("Current piece is %s\n", piece);
		int ino = search(current, piece);
		if (ino == 0){
			printf("Could not find %s\n", piece);
			return;
		}
		else{
			printf("Found %s with inode number %d\n", piece, ino);
		}
		blk = (ino - 1) / 8 + bg_inode_table;
		int offset = (ino - 1) % 8;
		//get the next inode
		get_block(fd, blk, buf);
		current = (INODE *)buf + offset;
	}
	parse_pathname(parameter, 1);
	//current->
	
	current->i_atime = current->i_ctime = time(0L);
	
	put_block(fd, blk, buf);
}

int my_chmod(){

	char buf[1024];
	printf("pathname = %s parameter = %s\n", pathname, parameter);
	short mode = atoi(pathname);

	if (mode == 0){
		printf("%s is not a new valid mode\n", pathname);
	}

	//are we relative to the root or relative to cwd?
	INODE *current = &root->INODE;
	if (*pathname != '/'){
		current = &running->cwd->INODE;
		//pathname++;
	}

	char *piece = "";
	int blk;
	while( piece = (char*)parse_pathname(parameter, 0) ){
		printf("Current piece is %s\n", piece);
		int ino = search(current, piece);
		if (ino == 0){
			printf("Could not find %s\n", piece);
			return;
		}
		else{
			printf("Found %s with inode number %d\n", piece, ino);
		}
		blk = (ino - 1) / 8 + bg_inode_table;
		int offset = (ino - 1) % 8;
		//get the next inode
		get_block(fd, blk, buf);
		current = (INODE *)buf + offset;
	}
	parse_pathname(parameter, 1);
	//change the permission of the current inode and write back to block
	//i_mode is the variable to change

	//566
	//566 / 100 = 5
	//

	short new_mode = (0 >> 9) << 9;
	
	short user  = mode / 100;
	short group = (mode % 100) / 10;
	short world = (mode % 100) % 10;
	
	/*
	
	new_mode |= (EXT2_S_IRUSR && nthBit(user, 2));
	new_mode |= (EXT2_S_IWUSR && nthBit(user, 1));
	new_mode |= (EXT2_S_IXUSR && nthBit(user, 0));
	new_mode |= (EXT2_S_IRGRP && nthBit(group, 2));
	new_mode |= (EXT2_S_IWGRP && nthBit(group, 1));	
	new_mode |= (EXT2_S_IXGRP && nthBit(group, 0));	
	new_mode |= (EXT2_S_IROTH && nthBit(world, 2));	
	new_mode |= (EXT2_S_IWOTH && nthBit(world, 1));	
	new_mode |= (EXT2_S_IXOTH && nthBit(world, 0));

	*/

	put_block(fd, blk, buf);
}














