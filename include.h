#ifndef INCLUDE_H
#define INCLUDE_H

#include <time.h>
#include "type.h"

typedef unsigned int u32;

extern int dev;
extern int ninodes;

extern MINODE minode[NMINODES];

extern char **BrokenDownPath(char *path, int *count);
extern int print_dir_entries(MINODE *mip);
extern MINODE *iget(int dev, int ino);
extern int get_block(int fd, int blk, char buf[ ]);
extern int search_inode(INODE * inodePtr, char * name);
extern int search_minode(MINODE *mip, char *name);
extern MINODE *iget(int dev, int ino);
extern int fd;
extern MINODE *root;
extern PROC *running;
extern u32 bg_inode_table;
extern char buf[BLOCK_SIZE];
extern MINODE minode[NMINODES];
extern int put_block(int fd, int blk, char buf[ ]);
extern int ialloc(int dev);

#endif
