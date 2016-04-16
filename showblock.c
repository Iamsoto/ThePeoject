//#include "include.h"	

char buf[BLOCK_SIZE];

//change shutup to dir

typedef unsigned int u32;

u32 bg_inode_table;

typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

GD    *gd;
SUPER *sp;
INODE *ip;
DIR   *dp; 

char *disk = "mydisk";

int print_superblock()
{
	printf("-----------------------------------------\n");
	printf("\t\tSUPERBLOCK INFORMATION\n");
	get_block(fd,1, buf);
	sp = (SUPER*)buf;

	if (sp->s_magic != 0xEF53){
    		printf("NOT an EXT2 FS\n");
    		exit(1);
  	}

  	printf("EXT2 FS OK\n");

	printf("s_inodes_count      = %d\n", sp->s_inodes_count);
	printf("s_blocks_count      = %d\n", sp->s_blocks_count);
	printf("s_free_blocks_count = %d\n", sp->s_free_blocks_count);
	printf("s_free_inodes_count = %d\n", sp->s_free_inodes_count);
	printf("s_first_data_block  = %d\n", sp->s_first_data_block);
	printf("s_log_block_size    = %d\n", sp->s_log_block_size);
	printf("s_blocks_per_group  = %d\n", sp->s_blocks_per_group);
	 
	printf("s_mnt_count         = %d\n", sp->s_mnt_count);
	printf("s_max_mnt_count     = %d\n", sp->s_max_mnt_count);

	printf("s_magic             = %.4x\n", sp->s_magic);
	
	printf("s_state             = %d\n", sp->s_state);
	printf("s_errors            = %d\n", sp->s_errors);
	printf("s_minor_rev_level   = %d\n", sp->s_minor_rev_level);
	printf("s_lastcheck         = %d\n", sp->s_lastcheck);
	printf("s_checkinterval     = %d\n", sp->s_checkinterval);
	printf("s_creator_os        = %d\n", sp->s_creator_os);
	printf("s_rev_level         = %d\n", sp->s_rev_level);
	printf("s_def_resuid        = %d\n", sp->s_def_resuid);
	printf("s_def_resgid        = %d\n", sp->s_def_resgid);
	printf("s_first_ino         = %d\n", sp->s_first_ino);
	printf("s_inode_size        = %d\n", sp->s_inode_size);
	
	printf("-----------------------------------------\n");
}

int showblock(char *path)
{
	get_block(fd, 2, buf);
	gd = (GD*)buf;

	bg_inode_table = gd->bg_inode_table;

	get_block(fd, bg_inode_table, buf);
	
	INODE *InodesBeginBlock = (INODE *)buf;
	
	if (InodesBeginBlock == 0)
		printf("Failed to find InodesBeginBlock\n");
	else
		printf("InodesBeginBlock is not null\n");	
	
	InodesBeginBlock += 1;//sample_dir->rec_len;
	
	return;

	int n = 0;
	char *entries[256] = { 0 };
	int i;

	char * s = strtok(path, "/");	
	
	if (s == 0) s = "/";

	printf("%s\n", s);

	entries[i] = s;
	i++;

	while (	s = strtok(0, "/") )
	{
		printf("%s\n", s);
		entries[i] = s;
		i++;
	}

	entries[i] = 0;
	n = i;

	printf("n = %d\n\n", n);

	getchar();

	int previous = -1;
	
	for (i = 0; i < n; i++)
	{
		char *current_name = entries[i]; 
		int res = search_inode(InodesBeginBlock,current_name);
		if (res == 0){ return printf("Could not find %s\n", path); } 
		else { //get the next one
			int next_inode = (res - 1) / 8 + bg_inode_table;
			int offset = (res - 1) % 8;
			get_block(fd, next_inode, buf);
			InodesBeginBlock = (INODE *)buf + offset;
		}
	}
}

int showblock_main(int argc, char *argv[])
{
	if (argc > 1)
	{
		disk = argv[1];
	}

	printf("Opening %s\n", disk);

	fd = open(disk, O_RDONLY);

	if (fd < 0)
	{
		printf("Open failed\n");
		exit(1);
	} 

	char path[1024];
	getcwd(path, 1024); 

	if (argc > 1) strcpy(path, argv[2]);

	print_superblock();
	showblock(path);

	return 0;
}
