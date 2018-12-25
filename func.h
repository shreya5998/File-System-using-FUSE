#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "proj_struct.h"
#include "disk.h"


struct srrfs
{
	struct super_block superblock;
	struct inode_stat inode[TOTAL_INODE_BLOCKS];
	union data_block dblock[TOTAL_DATA_BLOCKS];
};

struct srrfs fs;

int s_fd;
int i_fd;
int d_fd;

void OpenFS();

int diskFileSize(int fd);

void writeSuperBlock();
void readSuperBlock();
void PrintSB();

void writeInodeBlock();
void readInodeBlock();

void writeDataBlock(int , void *);
void readDataBlock(int , void *);

int LoadFS();
void WriteFS();

/*	Super Block Functions	*/
int createSuperblock();
int getFreeInodeNum();
int getFreeBlkNum();
int getCurrentInodeNum();
int getRootInode();
int getNextInodeNum();
int getBlockSize();
int getNextBlockNumber();
int getFreeBlock();

/*	Inode Functions		*/
int makeDir(const char *path,mode_t mode);
int createInode(const char *path , mode_t mode , type_t type);
int mapToInode(struct inode_stat inod , struct stat **stbuf);
int getInodeFileSize(int iNum);
int getInodeBlkAdr(int iNum);
int getInodeBlkNum(int iNum);
int getInodeNumFile(int parentINum,const char *path);
int getParentInodeNum(const char *path);
int getInodeNum(const char *path);
void incLink(int inodeNum);
void decLink(int inodeNum);
int updateInodeFileSize(int iNum , int size);


/*	Data Block Functions	*/
int addToDir(const char *path ,int iNum);
int addSelfAndParent(const char *path ,int iNum);
int getDirectoryIndex(int blknum);
void getFileName(const char *p,char path[]);
int getDirSize(int blknum);
int writeBlock(int blknum , const char *buf , int size , off_t offset);
int readBlock(int blknum , const char *buf , int size , off_t offset);
void updateDirSize(int blknum , int size);

