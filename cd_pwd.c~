#include "include.h"

extern char completePath[1024];




/**
 *
 */
char * totalPath(){

	char pathname[1024] = { 0 };
	char temp[50][100] = { { 0 } };
	char name0[124] ={ 0 };
	MINODE *mip = running->cwd;
	INODE * ip = 0;

	// Obtain the name of the mip
	if(strcmp(mip->name, "")==0){
		if(mip->ino> 2){
			// Assign a name to the bitch.
			DIR * dp  = (DIR*)getDir(&(mip->INODE), mip->ino );
			if(dp != 0){
			strcpy(name0, dp->name);
			}
		}
	}
	else {
		strcpy(name0, running->cwd->name);
	}

	strcpy(temp[0],name0);
	printf("The temp[0] = %s \n", temp[0]);

	// dp = Directory of minode
	// ip = parent node of mip



	int i=1;
	mip = (MINODE*)getParentMinode(mip, mip->ino);

	while(mip->ino != 2){

		//strcpy(temp[i], "/");
		strcpy(temp[i], mip->name);
		//printf("temp[i]=%s\n", temp[i]);
		mip = (MINODE*)getParentMinode(mip, mip->ino);

		i++;
		iput(mip);
	}

	if(mip != NULL){
		iput(mip);
	}

	// Add Slashes to total path
	strcpy(pathname, "/");
	if(strcmp("/", temp[0]) != 0){
		int j = 0;
		for (j = i; j > 0; j--){

			strcat(pathname, temp[j-1]);
			if( j-1 > 0){
				strcat(pathname, "/");
			}
		}
	}

	printf(" * *  * * The Pwd: %s * * * * * * *\n", pathname);

	int x =0;
	for(x =0; x< 1024; x++){
		completePath[x] = 0;
	}
	// Re-write the global variable:
	strcpy(completePath, pathname);

	return pathname;
}

int pwd(char *param){
	//printf(ANSI_COLOR_MAGENTA);
	totalPath();
	//printf(ANSI_COLOR_RESET);
	return 0;
}

int cd(char *param){
	int inum = getino(&dev, param);
	printf("The inum we've received is as follows:%d \n", inum);
	if(inum <2){
		printf("I do not believe that directory Exists. I'm sorry KC I can't let you do that \n");
		return 0;
	}
	MINODE* mip = iget(dev,inum);

	if(mip->name != "."){
		running->cwd = mip;
	}

	//printf(ANSI_COLOR_MAGENTA "Changed directories to: %s\n " ANSI_COLOR_RESET, running->cwd->name);
	printf("Changed directories to: %s\n", running->cwd->name);
	// Deallocate the refrence count
	iput(mip);
	printf("exited iput\n");
	fflush(stdin);
	return 0;
}
