#include "type.h"

/************************ globals *****************************/
MINODE *root;
char pathname[128], parameter[128], *name[128], cwdname[128];
char names[128][256];

int  nnames;
char *rootdev = "disk", *slash = "/", *dot = ".";
int iblock;

MINODE minode[NMINODES];
MOUNT  mounttab[NMOUNT];
PROC   proc[NPROC], *running;
OFT    oft[NOFT];

MOUNT *getmountp();

int DEBUG=0;
int nproc=0;

#include "include.h"   // include ALL YOUR .c files here

void mountroot()   /* mount root file system */
{
  int i, ino, fd, dev;
  MOUNT *mp;
  SUPER *sp;
  MINODE *ip;

  char line[64], buf[BLOCK_SIZE], *rootdev;
  int ninodes, nblocks, ifree, bfree;

  printf("enter rootdev name (RETURN for disk) : ");
  gets(line);

  rootdev = "disk";

  if (line[0] != 0)
     rootdev = line;

  dev = open(rootdev, O_RDWR);
  if (dev < 0){
     printf("panic : can't open root device\n");
     exit(1);
  }

  /* get super block of rootdev */
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* check magic number */
  printf("SUPER magic=0x%x  ", sp->s_magic);
  if (sp->s_magic != SUPER_MAGIC){
     printf("super magic=%x : %s is not a valid Ext2 filesys\n",
             sp->s_magic, rootdev);
     exit(0);
  }

  mp = &mounttab[0];      /* use mounttab[0] */

  /* copy super block info to mounttab[0] */
  ninodes = mp->ninodes = sp->s_inodes_count;
  nblocks = mp->nblocks = sp->s_blocks_count;

  bfree = sp->s_free_blocks_count;
  ifree = sp->s_free_inodes_count;

  get_block(dev, 2, buf);
  gp = (GD *)buf;

  mp->dev = dev;
  mp->busy = BUSY;

  mp->bmap = gp->bg_block_bitmap;
  mp->imap = gp->bg_inode_bitmap;
  mp->iblock = gp->bg_inode_table;

  strcpy(mp->name, rootdev);
  strcpy(mp->mount_name, "/");


  printf("bmap=%d  ",   gp->bg_block_bitmap);
  printf("imap=%d  ",   gp->bg_inode_bitmap);
  printf("iblock=%d\n", gp->bg_inode_table);


  /***** call iget(), which inc the Minode's refCount ****/

  root = iget(dev, 2);          /* get root inode */
  mp->mounted_inode = root;
  root->mountptr = mp;

  printf("mount : %s  mounted on / \n", rootdev);
  printf("nblocks=%d  bfree=%d   ninodes=%d  ifree=%d\n",
	  nblocks, bfree, ninodes, ifree);

  return(0);
}

init()
{
  int i, j;
  PROC *p;

  for (i=0; i<NMINODES; i++)
      minode[i].refCount = 0;

  for (i=0; i<NMOUNT; i++)
      mounttab[i].busy = 0;

  for (i=0; i<NPROC; i++){
      proc[i].status = FREE;
      for (j=0; j<NFD; j++)
          proc[i].fd[j] = 0;
      proc[i].next = &proc[i+1];
  }

  for (i=0; i<NOFT; i++)
      oft[i].refCount = 0;

  printf("mounting root\n");
    mountroot();
  printf("mounted root\n");

  printf("creating P0, P1\n");
  p = running = &proc[0];
  p->status = BUSY;
  p->uid = 0;
  p->pid = p->ppid = p->gid = 0;
  p->parent = p->sibling = p;
  p->child = 0;
  p->cwd = root;
  p->cwd->refCount++;

  p = &proc[1];
  p->next = &proc[0];
  p->status = BUSY;
  p->uid = 2;
  p->pid = 1;
  p->ppid = p->gid = 0;
  p->cwd = root;
  p->cwd->refCount++;

  nproc = 2;
}

int quit()
{
  // write YOUR quit function here
   exit(0);
}

main(int argc, char *argv[ ])
{
  int i,cmd;
  char line[128], cname[64];

  if (argc>1){
    if (strcmp(argv[1], "-d")==0)
        DEBUG = 1;
  }

  init();

  while(1){
      printf("P%d running: ", running->pid);

      /* zero out pathname, parameter */
      for (i=0; i<64; i++){
          pathname[i]=parameter[i] = 0;
      }
      /* these do the same; set the strings to 0 */
      memset(pathname, 0, 64);
      memset(parameter,0, 64);

      printf("input command : ");
      gets(line);
      if (line[0]==0) continue;

      sscanf(line, "%s %s %64c", cname, pathname, parameter);

      cmd = findCmd(cname);
      switch(cmd){
           case 0 : make_dir();               break;
           case 1 : change_dir();             break;
           case 2 : pwd(running->cwd);        break;
           case 3 : list_dir();               break;
           case 4 : mount();                  break;
           case 5 : umount(pathname);         break;
           case 6 : creat_file();             break;
           case 7 : rmdir();                  break;
           case 8 : rm_file();                break;
           case 9 : open_file();              break;
           case 10: close_file();             break;

           case 11: read_file();              break;
           case 12: write_file();             break;
           case 13: cat_file();               break;

           case 14: cp_file();                break;
           case 15: mv_file();                break;

           case 16: pfd();                    break;
           case 17: lseek_file();             break;
           case 18: rewind_file();            break;
           case 19: mystat();                 break;

           case 20: pm();                     break;

           case 21: menu();                   break;

           case 22: access_file();            break;
           case 23: chmod_file();             break;
           case 24: chown_file();             break;

           case 25: cs();                     break;
           case 26: do_fork();                break;
           case 27: do_ps();                  break;
           case 28: do_kill();                break;

           case 29: quit();                   break;
           case 30: do_touch();               break;

           case 31: sync();                   break;
           case 32: link(); break;
           case 33: unlink(); break;
           case 34: symlink(); break;
           default: printf("invalid command\n");
                    break;
      }
  }
} /* end main */

// NOTE: you MUST use a function pointer table




