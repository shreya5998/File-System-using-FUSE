#include "disk.h"
/*	Function to open disk and returns the file descriptor	*/
int OpenDisk(char *filename , int nbytes)
{
	printf("Opening file %s \n",filename);
	int fd;
	fd = open(filename , O_RDWR | O_CREAT , 0664);
	
	/*	Lseek to increase the size of the file by nbytes	*/
	int result = lseek(fd, nbytes, SEEK_SET);
	if (result == -1) 
	{
		perror("Error calling lseek() to 'stretch' the file");
		return -1;
    }
	return fd;	
}

/*	Function to read the block and store in buf	*/
int ReadBlock(int fd ,  int blknum , void *buf)
{
	if(pread(fd , buf ,BLOCK_SIZE, blknum*BLOCK_SIZE)<0)
			perror("File ");
}


/*	Function to write into the block from buf	*/
int WriteBlock(int fd , int blknum, void *buf )
{
	if(pwrite(fd , buf , BLOCK_SIZE , blknum*BLOCK_SIZE)<0)
		perror("Write");
}
