#include "func.h"

/*	Function to open the files of Disk	*/
void OpenFS()
{
	printf("OpenFS function called \n");
	s_fd = OpenDisk("superblock", BLOCK_SIZE);
	i_fd = OpenDisk("inode" , BLOCK_SIZE);
	d_fd = OpenDisk("datablock" , BLOCK_SIZE * TOTAL_DATA_BLOCKS);
}

/*	Function to load the file system saved in a file to memory	*/
int LoadFS()
{
	printf("LoadFS called \n");
	readSuperBlock();	/*	Load superblock		*/
	readInodeBlock();	/*	Load inode blocks	*/
	if (diskFileSize(d_fd) != 0)	/*	Load data blocks	*/
	{
		for (int i = 0 ; i < getFreeBlock() ; ++i)
		{
			readDataBlock(i , &(fs.dblock[i]));
		}
	}
}

/*	Function to save the file system into secondary memory	*/
void WriteFS()
{
	printf("WriteFS called \n");
	writeSuperBlock();
	writeInodeBlock();
}

/*	Function to check the size of the file (disk file)	*/
int diskFileSize(int fd)
{
	off_t fsize;
	fsize = lseek(fd, 0, SEEK_END);
	return fsize;
}

/*	Function to write SuperBlock into the secondary memory	*/
void writeSuperBlock()
{
	printf("WriteSuperBlock called \n");
	WriteBlock(s_fd , 0 , &(fs.superblock));
}

/*	Function to read SuperBlock from the secondary memory	*/
void readSuperBlock()
{
	printf("ReadSuperBlock called \n");
	/*	Check if the file system exists in secondary memory	*/
	if (diskFileSize(s_fd) == 0)
	{
		/*	Create SuperBlock in memory	*/
		createSuperblock();
		/*	Create root Directory	*/
		int ret = makeDir("/", 0775);
		/*	If unable to create root directory	*/
		if (ret == -1)
		{
			fprintf(stderr , "Unable to make a file system \n");
			return;
		}
		/*	Write into secondary memory	*/
		writeSuperBlock();
	}
	else
	{
		/*	Read from secondary memory	*/
		ReadBlock(s_fd , 0 , &(fs.superblock));
	}
	//PrintSB();
}

/*	Function to display details of superblock (Testing purpose)	*/
void PrintSB()
{
	printf("Block Size = %d \n", (fs.superblock).block_size);
	printf("Number of data blocks = %d\n", (fs.superblock).num_blocks);
	printf("Number of inode blocks = %d\n", (fs.superblock).num_inode);
	printf("Number of free inode blocks = %d\n", (fs.superblock).num_free_inode);
	printf("Number of free data blocks = %d\n", (fs.superblock).num_free_blocks);
	printf("Free Data block head = %d\n", (fs.superblock).free_block_head);
	printf("Current inode number = %d\n", (fs.superblock).current_inode_no);
	//printf("\n\n Data block size = %d \n",sizeof(fs.dblock[0]));
}

/*	Function to write inode blocks into secondary memory	*/
void writeInodeBlock()
{
	printf("writeInodeBlock called \n");
	WriteBlock(i_fd , 0 , &(fs.inode));
}

/*	Function to read inode blocks from secondary memory	*/
void readInodeBlock()
{
	printf("ReadInodeBlock called \n");
	if (diskFileSize(i_fd) != 0)
	{
		ReadBlock(i_fd , 0 , &(fs.inode));
		/*printf("inode 1 st_ino = %d \n",fs.inode[0].st_ino);
		for(int i= 0 ; i<=getCurrentInodeNum(); ++i)
		{
			printf("inode %d st_blkadr = %d \n",i , fs.inode[i].st_blkadr);
		}*/
	}
}

/*	Function to write specified datablock into the secondary memory	*/
void writeDataBlock(int blknum , void *buf)
{
	printf("writeDataBlock called with blknum = %d\n", blknum);
	WriteBlock(d_fd , blknum , buf);
}

/*	Function to read specified datablock from the secondary memory	*/
void readDataBlock(int blknum , void *buf)
{
	printf("ReadDataBlock called with blknum = %d\n", blknum);
	if (diskFileSize(d_fd) != 0)
	{
		ReadBlock(d_fd , blknum , buf);
	}
}

int createSuperblock()
{

	(fs.superblock).block_size = BLOCK_SIZE; //6
	(fs.superblock).num_blocks = TOTAL_DATA_BLOCKS;
	(fs.superblock).num_free_blocks = TOTAL_DATA_BLOCKS;
	(fs.superblock).num_inode = TOTAL_INODE_BLOCKS;
	(fs.superblock).num_free_inode = TOTAL_INODE_BLOCKS;
	(fs.superblock).free_block_head = 0;
	(fs.superblock).root_ino = 0;
	(fs.superblock).current_inode_no = -1;
	return 0;

}


/*	Function to return the number of free inodes	*/
int getFreeInodeNum()
{
	return (fs.superblock).num_free_inode ;
}

/*	Function to return the number of free blocks	*/
int getFreeBlkNum()
{
	return (fs.superblock).num_free_blocks ;
}


/*	Function to return the current inode number	*/
int getCurrentInodeNum()
{
	return (fs.superblock).current_inode_no ;
}

/*	Function to get the next free inode number	*/
int getNextInodeNum()
{
	/*	Check if there are free inodes remaining	*/
	if (getFreeInodeNum() == 0)
		return -1;
	/*	Decrement number of free inodes */
	(fs.superblock).num_free_inode -= 1;
	fs.superblock.current_inode_no += 1;
	return fs.superblock.current_inode_no;
}

/*	Function to get the next free block number	*/
int getNextBlockNumber()
{
	/*	Check if there are free blocks remaining	*/
	if (getFreeBlkNum() == 0)
		return -1;
	/*	Decrement number of free data blocks	*/
	fs.superblock.num_free_blocks -= 6;
	int num =  (fs.superblock).free_block_head;
	/*	Update the free blocks header	*/
	(fs.superblock).free_block_head += 6;
	return num;
}

/*	Function to get the free block number	*/
int getFreeBlock()
{
	if (getFreeBlkNum() == 0)
		return -1;
	return (fs.superblock).free_block_head;
}
/*	Function to return the root inode number	*/
int getRootInode()
{
	return fs.superblock.root_ino;
}

/*	Function to get the block size of the file system	*/
int getBlockSize()
{
	return fs.superblock.block_size;
}

/*	Inode Functions		*/

/*Function to create an Inode for a file	*/
int createInode(const char *path , mode_t mode , type_t type)
{
	/*	Get the next free inode number	*/
	int i = getNextInodeNum();
	if (i == -1)
	{
		fprintf(stderr , "Maximum Files Size Reached\n");
		return -1;
	}
	/*	Update inode values	*/
	fs.inode[i].st_ino = i;
	fs.inode[i].st_mode = mode;
	fs.inode[i].st_type = type;
	fs.inode[i].st_uid = getuid();
	fs.inode[i].st_gid = getgid();
	fs.inode[i].st_blknum = 6;
	int blknum = getNextBlockNumber();
	if (blknum == -1)
	{
		fprintf(stderr , "Maximum blocks reached\n");
		return -1;
	}
	fs.inode[i].st_blkadr = blknum;

	/*	Directory File	*/
	if (type == Directory)
	{
		fs.inode[i].st_nlink = 2;
		fs.inode[i].st_size = 4096;
		fs.dblock[blknum].dir.size = 0;		/*	Assign number of directory entries to 0	*/
		//printf("Allocated size of directory = %d \n",fs.dblock[blknum].dir.size);
	}
	/*	Regular File	*/
	else
	{
		fs.inode[i].st_nlink = 1;
		fs.inode[i].st_size = 0;
	}
	return i;			/*	Returns inode of the file created	*/
}

/*	Function to return the starting block address of the specified inode (iNum)	*/
int getInodeBlkAdr(int iNum)
{
	if (getCurrentInodeNum() < iNum)	/* Check if the inode exists	*/
		return -1;
	return fs.inode[iNum].st_blkadr;
}

/*	Function to increment the number of hard links of specified inode(iNum) by 1	*/
void incLink(int inodeNum)
{
	fs.inode[inodeNum].st_nlink += 1;
}

/*	Function to decrement the number of hard links of specified inode(iNum) by 1	*/
void decLink(int inodeNum)
{
	fs.inode[inodeNum].st_nlink -= 1;
}

/*	Function to return the file size of the specified inode (iNum)	*/
int getInodeFileSize(int iNum)
{
	if (getCurrentInodeNum() < iNum)	/* Check if the inode exists	*/
		return -1;
	return fs.inode[iNum].st_size;
}


/*	Function to update the file size of the specfied inode (iNum)	*/
int updateInodeFileSize(int iNum , int size)
{
	fs.inode[iNum].st_size = size;
}

/*	Function to return the number of blocks of the specified inode (iNum)	*/
int getInodeBlkNum(int iNum)
{
	if (getCurrentInodeNum() < iNum)	/* Check if the inode exists	*/
		return -1;
	return fs.inode[iNum].st_blknum;
}

/*	Function to create a directory	*/
int makeDir(const char *path, mode_t mode)
{
	printf("makeDir called for path = %s \n", path);
	int inodeNum = createInode(path , mode , Directory);
	/*	If unable to create inode	*/
	if (inodeNum == -1)
	{
		return -1;
	}
	if (strcmp(path, "/") == 0)
	{
		printf("Successfully created / file \n");
	}
	else
	{
		addToDir(path, inodeNum);
		addSelfAndParent(path , inodeNum);
		printf("Successfully created Directory %s\n", path);
	}

	return 0;
}


/*	Data Block Functions	*/

/*	Function to add a specified file name to the directory entry	*/
int addToDir(const char *path , int iNum)
{
	char fileName[50] ;
	/*	Get the parent directory inode number	*/
	int parentInode = getParentInodeNum(path);
	/*	Get name of the file only from the path	*/
	getFileName(path, fileName);
	if (parentInode == -1)
		return -1;
	int blkadr = getInodeBlkAdr(parentInode);
	int i = getDirectoryIndex(blkadr);

	/*	Update the file entry into the directory */
	fs.dblock[blkadr].dir.dent[i].st_ino = iNum;
	strcpy(fs.dblock[blkadr].dir.dent[i].f_name, fileName);

	/*	Increment the number of links of the parent directory	*/
	incLink(parentInode);
	/*	Write into the secondary memory	*/
	writeDataBlock(blkadr , &(fs.dblock[blkadr]));
	//printf("Added %s to parentInode %d with blkadr %d at index %d with size = %d\n",fs.dblock[blkadr].dir.dent[i].f_name , parentInode,blkadr,i,fs.dblock[blkadr].dir.size);
	return 0;
}

/*	Function to add "." and ".." as the directory entry	*/
int addSelfAndParent(const char *path , int iNum)
{
	int parentInode = getParentInodeNum(path);
	char fileName[50] ;
	getFileName(path, fileName);
	if (parentInode == -1)
		return -1;
	int blkadr = getInodeBlkAdr(iNum);

	/*	Add "." and ".." into the entry of the directory	*/
	int i = getDirectoryIndex(blkadr);
	fs.dblock[blkadr].dir.dent[i].st_ino = iNum;
	strcpy(fs.dblock[blkadr].dir.dent[i].f_name, ".");

	i = getDirectoryIndex(blkadr);
	fs.dblock[blkadr].dir.dent[i].st_ino = parentInode;
	strcpy(fs.dblock[blkadr].dir.dent[i].f_name, "..");
	/*	Write into the secondary memory	*/
	writeDataBlock(blkadr , &(fs.dblock[blkadr]));
	return 0;
}

/*	Function to get the directory inode from the given parent inode number	*/
int getInodeNumFile(int parentINum, const char *path)
{
	int i;
	int blkadr = getInodeBlkAdr(parentINum);
	int size = getDirSize(blkadr);
	for (i = 0; i < size ; ++i)
	{
		/*	Check if the file entry exists	*/
		if (strcmp(fs.dblock[blkadr].dir.dent[i].f_name , path) == 0)
			return fs.dblock[blkadr].dir.dent[i].st_ino;
	}
	return -1;
}

/*	Function to get the parent inode number of the specified file	*/
int getParentInodeNum(const char *path1)
{
	char path[MAX_FILE_LENGTH];
	strcpy(path, path1 + 1);
	int len = strlen(path);
	/*	Initialise iNum by the root inode number	*/
	int iNum = getRootInode();
	int i = 0;
	int j = 0;
	char dname[MAX_FILE_LENGTH];
	while (i < len)
	{
		if (path[i] == '/')
		{
			dname[j] = '\0';
			j = 0;
			/*	Update the inode number of the parent	*/
			iNum = getInodeNumFile(iNum , dname);
			if (iNum == -1)
				return -1;
		}
		else
		{
			dname[j] = path[i];
			++j;
		}
		++i;
	}
	return iNum;
}

/*Function to return the inode number of the specified file	*/
int getInodeNum(const char *path)
{
	char fname[MAX_FILE_LENGTH];
	if (strcmp(path, "/") == 0)
	{
		return getRootInode();
	}
	/*	Get parent inode number	*/
	int parentInode = getParentInodeNum(path);
	if (parentInode == -1)
		return -1;
	/*	Get file name only from the path	*/
	getFileName(path , fname);

	int blkadr = getInodeBlkAdr(parentInode);
	int size = getDirSize(blkadr);
	for (int i = 0; i < size ; ++i)
	{
		/*	Check if the file entry exists	*/
		if (strcmp(fs.dblock[blkadr].dir.dent[i].f_name , fname) == 0)
			return fs.dblock[blkadr].dir.dent[i].st_ino;
	}

	return -1;
}

/*	Function to return the next index of the directory entry of the specified directory	*/
int getDirectoryIndex(int blkadr)
{
	int size = getDirSize(blkadr);
	fs.dblock[blkadr].dir.size += 1 ;
	return size;
}

/*	Function to get the filename from the specified absolute path */
void getFileName(const char *p, char fname[])
{
	int len;
	int index;
	char path[50] ;

	strcpy(path , p);
	len = strlen(path);
	index = len - 1;


	while (index >= 0 && path[index] != '/')
	{
		fname[(len - 1) - index] = path[index]; 		/*	Fname gets stored in reverse way	*/
		index -= 1;
	}
	if (index >= 0)
		fname[(len - 1) - index] = '\0';

	len = strlen(fname);
	/*	Reverse the fname to get the actual filename	*/
	for (int i = 0 ; i < len / 2; ++i)
	{
		char tmp = fname[i];
		fname[i] = fname[len - i - 1];
		fname[len - i - 1] = tmp;
	}
	//printf("File name = %s \n",fname);
}

/*	Function to get the directory file size		*/
int getDirSize(int blknum)
{
	return fs.dblock[blknum].dir.size;
}


/*	Function to write the data into the specified block (blkNum)	*/
int writeBlock(int blknum , const char *buf , int size , off_t offset)
{
	int blksize = getBlockSize();
	if (offset == 0)
	{
		fs.dblock[blknum].data = (char *)malloc(sizeof(char) * blksize);
		strncpy(fs.dblock[blknum].data, buf, size);
	}
	else
	{
		return -1;
	}
	return 0;
}

/*	Function to read the data of the specified block (blkNum)	*/
int readBlock(int blknum , const char *buf , int size , off_t offset)
{
	if (offset == 0)
	{
		strncpy((char *)buf, fs.dblock[blknum].data, size);
	}
	else
	{
		return -1;
	}
	return 0;
}


/*	Function to update directory size by size(positive or negative)*/
void updateDirSize(int blkadr , int size)
{
	fs.dblock[blkadr].dir.size = size;
}
