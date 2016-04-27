#include "include.h"

extern int idealloc(int dev, int ino);
extern int bdealloc(int dev, int bno);
extern int my_link(char *pathname, char *parameter);
extern int my_unlink(char *ptn);

/*
	                      HOW TO cp ONE file:
	cp src dest:

	1. fd = open src for READ;

	2. gd = open dst for WR|CREAT; 

	   NOTE:In the project, you may have to creat the dst file first, then open it 
		for WR, OR  if open fails due to no file yet, creat it and then open it
		for WR.

	3. while( n=read(fd, buf[ ], BLKSIZE) ){
	       write(gd, buf, n);  // notice the n in write()
	   }
*/

int my_cp(){

	//command_name pathname parameter

	char buf[1024] = { 0 };

	int fd, gd;
	printf("Copying %s to %s\n", pathname, parameter);
	fd = laopen_file(pathname,  "0");
	gd = laopen_file(parameter, "2");
	
	printf("fd = %d gd = %d\n", fd, gd);
	getchar();
	
	int n;
	while(n = laread(fd, buf, 1024)){
		printf("n = %d\n", n);
		buf[n] = 0;
		getchar();
		sprintf(pathname, "%d", gd);
		strcpy(parameter, buf);
		my_write(gd, buf, n);
	}
	
	laclose_file(fd);
	laclose_file(gd);
}

/*

		HOW TO mv (rename)
mv src dest:

1. verify src exists; get its INODE in ==> you already know its dev
2. check whether src is on the same dev as src

              CASE 1: same dev:
3. Hard link dst with src (i.e. same INODE number)
4. unlink src (i.e. rm src name from its parent directory and reduce INODE's
               link count by 1).
                
              CASE 2: not the same dev:
3. cp src to dst
4. unlink src

*/

int mv(){
	//pathname, parameter
	char ptn[256];
	strcpy(ptn, pathname);
	char param[256];
	strcpy(param, parameter);
	
	int dev;
	int src_ino  = getino(&dev,  pathname);
	int dev2;
	int dest_ino = getino(&dev2, parameter);
	
	if (src_ino == 0){
		printf("%s does not exists, cancelling\n", pathname);
		return;
	}
	
	if (dev == dev2){
		//pathname = (char*)param;
		printf("same dev\n");
		printf("ptn = %s pathname = %s\n", ptn, param);
		getchar();
		my_link(ptn, param);
		printf("ptn = %s pathname = %s\n", ptn, param);
		getchar();
		my_unlink(pathname);
	}else{ //umount and mount not implemented, unlikely to be used
	
	}
	
}









