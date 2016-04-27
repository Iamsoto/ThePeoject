#include "include.h"

#define EXT2_S_IRUSR	0x0100	
#define EXT2_S_IWUSR	0x0080	
#define EXT2_S_IXUSR	0x0040	
#define EXT2_S_IRGRP	0x0020	
#define EXT2_S_IWGRP	0x0010	
#define EXT2_S_IXGRP	0x0008	
#define EXT2_S_IROTH	0x0004	
#define EXT2_S_IWOTH	0x0002	
#define EXT2_S_IXOTH	0x0001

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
	int dev;
	int param_ino    = my_getino(&dev, parameter);
	int param_block  = (param_ino - 1) / 8 + bg_inode_table;
	int param_offset = (param_ino - 1) % 8;
	
	get_block(dev, param_block, buf);
	INODE *current = (INODE*)buf + param_offset;
	
	current->i_uid = uid;
	put_block(fd, param_block, buf);
	
	printf("exiting chown\n");
}

int my_stat(char *pathname, struct stat *st){
	char buf[1024];
	int dev;
	int ptn_ino      = my_getino(&dev, pathname);
	int ptn_blk      = (ptn_ino - 1) / 8 + bg_inode_table;
	int ptn_offset   = (ptn_ino - 1) % 8;
	get_block(dev, ptn_blk, buf);
	INODE *ptn_inode = (INODE*)buf + ptn_offset;
	
	(*st).st_dev     = dev;
	(*st).st_ino     = ptn_ino;
	(*st).st_mode    = ptn_inode->i_mode;
	(*st).st_nlink   = 0; //TODO
	(*st).st_uid     = ptn_inode->i_uid;
	(*st).st_gid     = ptn_inode->i_gid;
	(*st).st_rdev    = dev;
	(*st).st_size    = ptn_inode->i_size;
	(*st).st_atime   = ptn_inode->i_atime;
	(*st).st_mtime   = ptn_inode->i_mtime;
	(*st).st_ctime   = ptn_inode->i_ctime;
	(*st).st_blksize = 1024;
	int alloc_size   = 0;
	int i;
	for (i = 0; i < 15; i++){
		if (ptn_inode->i_block[i] != 0){
			alloc_size++;
		}
	}
	(*st).st_blocks  = alloc_size; 
}

int touch(char *pathname){
	char buf[1024];
	//are we relative to the root or relative to cwd?
	INODE *current;
	
	int dev;
	int ino    = getino(&dev, parameter);
	int blk    = (ino - 1) / 8 + bg_inode_table;
	int offset = (ino - 1) % 8;
	
	get_block(dev, blk, buf);
	
	current->i_atime = current->i_ctime = time(0L);
	
	put_block(dev, blk, buf);
}

int my_chmod(){

	char buf[1024];
	printf("pathname = %s parameter = %s\n", pathname, parameter);
	short mode = atoi(pathname);

	if (mode == 0){
		printf("%s is not a new valid mode\n", pathname);
	}
	
	int dev;
	int ino = my_getino(&dev, parameter);
	int blk = (ino - 1) / 8 + bg_inode_table;
	int offset = (ino - 1) % 8;
	get_block(dev, blk, buf);
	
	INODE *current = (INODE*)buf + offset;

	short user  = pathname[0] - '0'; user  <<= 6;
	short group = pathname[1] - '0'; group <<= 3;
	short new_mode = pathname[2] - '0';
	
	new_mode |= group;
	new_mode |= user;
	new_mode |= (current->i_mode >> 9) << 9;
	
	current->i_mode = new_mode;
	
	put_block(fd, blk, buf);
}














