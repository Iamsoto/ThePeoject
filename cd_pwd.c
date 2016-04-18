#include "include.h"

char *totalPath(){
	char pathname[1024] = { 0 };
	char temp[50][100] = { { 0 } };
	char name0[124] ={ 0 };
	MINODE *mip = running->cwd;

	// Having issues where running->cwd->name is null
	// I believe this is because when its ref count hits 0 the name gets reset
	// This is because KC's getino sets pre-existing minodes without resetting the name
	// So iput has to reset the name instead... not a pretty way to do this
	if(strcmp(mip->name, "")==0){
		if(mip->ino> 2){
			// Assign a name to the bitch.
			DIR * dp  = getDir(&(mip->INODE), mip->ino );
			if(dp != 0){
			printf("DO THE BOOGIE! \n");
			strcpy(name0, dp->name);
			}
		}
	}
	else {
		strcpy(name0, running->cwd->name);
	}

	strcpy(temp[0],name0);
	printf("The temp[0] = %s \n", temp[0]);
	iput(mip);
	int i=1;
	while(mip->ino != 2){
		mip = getParentNode(mip, mip->ino);
		printf("The parent Node name is: %s, and the ino is: %d\n", mip->name, mip->ino);
		//strcpy(temp[i], "/");
		strcpy(temp[i], mip->name);
		//printf("temp[i]=%s\n", temp[i]);
		i++;
		iput(mip);
	}
	int j =0;
	strcpy(pathname, " ");
	for (j = i; j > 0; j--){
		strcat(pathname, temp[j-1]);
		strcat(pathname, "/");
	}
	printf("The path: %s\n", pathname);
	// place the pathname in param. This is kinda cheating but...
	// Oh well


	return pathname;
}

int pwd(char *param){
	totalPath();
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

	printf("Changed directories to: %s\n ", running->cwd->name);
	// Deallocate the refrence count
	iput(mip);
	printf("exited iput\n");
	fflush(stdin);
	return 0;
}
