#include "func.h"
#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>


// struct filehandle {
// 	struct inode_stat *node;
// 	int o_flags;
// };


// #define U_ATIME (1 << 0)
// #define U_CTIME (1 << 1)
// #define U_MTIME (1 << 2)



// static void update_times(int inodeNum, int which) {
// 	time_t now = time(0);
// 	if (which & U_ATIME) fs.inode[inodeNum].st_atim = now;
// 	if (which & U_CTIME) fs.inode[inodeNum].st_ctim = now;
// 	if (which & U_MTIME) fs.inode[inodeNum].st_mtim = now;
// }

int makefs()
{
	printf("makefs called \n");
	/*	Opening disk files	*/
	OpenFS();
	/*	Load disk files		*/	
	LoadFS();
	printf("mkfs Successful \n");
	return 0;
}
int mapToInode(struct inode_stat inod , struct stat **stbuf)
{
	if (stbuf == NULL)
	{
		char msg[] = "Error : Invalid Inode \n";
		write(0 , msg , strlen(msg));
		return -1;
	}
	//printf("Inode Number = %d",inod.st_ino);
	(*stbuf)->st_ino = inod.st_ino;
	(*stbuf)->st_uid = inod.st_uid;
	(*stbuf)->st_gid = inod.st_gid;
	(*stbuf)->st_size = inod.st_size;
	(*stbuf)->st_blksize = getBlockSize();
	(*stbuf)->st_blocks = inod.st_blknum;
	(*stbuf)->st_atime =  time(NULL);
	(*stbuf)->st_mtime =  time(NULL);
	(*stbuf)->st_ctime =  time(NULL);
	(*stbuf)->st_nlink = inod.st_nlink;
	/*	Regular File	*/
	if (inod.st_type == Regular)
		(*stbuf)->st_mode = S_IFREG | inod.st_mode;
	/*	Directory File	*/
	else if (inod.st_type == Directory)
		(*stbuf)->st_mode = S_IFDIR | inod.st_mode;
	return 0;
}
static int do_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("Creating file %s \n",path);
	/*	Check if the file already exists	*/
	int inodeNum = getInodeNum(path);	
	if(inodeNum != -1)
		-ENOENT;
	/*	Create an Inode for the regular file	*/	
	inodeNum = createInode(path , mode , Regular);
	if(inodeNum == -1)
	{
		return -1;
	}	
	else
	{
		/*	Add the file name to the directory entry	*/
		addToDir(path,inodeNum);	
		printf("Successfully created file %s\n",path);		
	}
	WriteFS();
	//readSuperBlock();
	return 0;
}

static int do_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{

	printf("Getting Attributes of %s \n", path);
	/*	Get the required inode number	*/
	int iNum = getInodeNum(path);
	if (iNum == -1)
		return -ENOENT;
	/*	Convert inode to Stat	*/
	mapToInode(fs.inode[iNum], &stbuf);
	return 0;
}

static int do_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	printf("Displaying files of path %s \n", path);
	int rootInode = getRootInode();
	int blkadr = getInodeBlkAdr(rootInode);
	int size = getDirSize(blkadr);
	/*	Root file	*/
	if (strcmp(path, "/") == 0)
	{
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);

		for (int i = 0; i < size ; ++i)
		{
			if (fs.dblock[blkadr].dir.dent[i].st_ino == -1) continue;
			filler(buf , fs.dblock[blkadr].dir.dent[i].f_name, NULL, 0);
		}
	}
	/*	Any file other than root	*/
	else
	{
		int parentInode = getInodeNum(path);
		int blkadr = getInodeBlkAdr(parentInode);
		int size = getDirSize(blkadr);
		for (int i = 0; i < size ; ++i)
		{
			if (fs.dblock[blkadr].dir.dent[i].st_ino == -1) continue;
			filler(buf , fs.dblock[blkadr].dir.dent[i].f_name, NULL, 0);
		}
	}
	return 0;
}


static int do_mkdir(const char *path, mode_t mode)
{
	printf("Make directory called with path newest%s\n", path);
	int inodeNum = getInodeNum(path);
	if (inodeNum != -1)
		return -ENOENT;
	int ret = makeDir(path , mode);
	if (ret == -1)
		return -1;
	return 0;

}
// static int do_open(const char *path, struct fuse_file_info *fi)
// {
// 	int inodeNum = getInodeNum(path);
// 	update_times(inodeNum, U_ATIME);
// 	struct filehandle *fh = malloc(sizeof(struct filehandle));
// 	fh->node    = inodeNum;
// 	fh->o_flags = fi->flags;
// 	fi->fh = (uint64_t) fh;
// 	fs.inode[inodeNum].st_nlink += 1;
// 	return 0;
// }
static int do_open(const char *path, struct fuse_file_info *fi)
{
	printf("Opening file \n");
	/*	Check if the file exists or not		*/
	if(getInodeNum(path) == 0)	
		return -ENOENT;
	if(fi->flags & O_ACCMODE == O_RDONLY)
		printf("Opened for reading purpose\n");	
	else if(fi->flags & O_ACCMODE == O_WRONLY)
		printf("Opened for reading and writing purpose\n");	

	return 0;
}
static int do_write(const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{
	printf("Write Called with size %ld , offset %ld\n",size, offset);
	/*	For offset equal to 0	*/
	if(offset == 0)
	{
		int iNum = getInodeNum(path);	/*	Get inode number	*/
		/*	Check if the file exists	*/
		if(iNum == -1)
			return -ENOENT;
		int blkadr = getInodeBlkAdr(iNum);
		int blknum = getInodeBlkNum(iNum);
		int blksize = getBlockSize();
		int i = 0;
		int remSize = size;
		do
		{
			/*Write buffer data to the data block	*/
				writeDataBlock(blkadr+i ,(char *)&buf[i*blksize]);
			/*	If size of data left to write is less than block size	*/ 
			/*if(remSize > blksize)
			{
				//writeBlock(blkadr+i , &buf[i*blksize] , blksize , offset);
			}	
			else	
			{
				writeDataBlock(blkadr+i ,&buf[i*blksize]);
				//writeBlock(blkadr+i , &buf[i*blksize] , remSize , offset);
			}*/
			++i;
			remSize = remSize - blksize;
		}while(i<blknum && remSize>0);

		/*	If the data written is less than maximum size of the file	*/
		if(i < blknum)	
			updateInodeFileSize(iNum , size);
		/*	If the data written exceeds the maximum size of the file	*/
		else	
			updateInodeFileSize(iNum ,blksize * i);
		
		/*	Update Superblock and inode data	*/
		WriteFS();
		
		return getInodeFileSize(iNum);

	}
	else
	{
		int iNum = getInodeNum(path);
		if(iNum == -1)
			return -ENOENT;
		int blkadr = getInodeBlkAdr(iNum);
		int blknum = getInodeBlkNum(iNum);
		int blksize = getBlockSize();
		int remSize = size;
		int inodesize = getInodeFileSize(iNum);
		int i = 0;
		/*	Find the block to be written	*/
		while(i < blknum && (offset > 0 && offset > blksize))
		{
			offset = offset - blksize; 
			//blkadr += 1;
			i += 1;
		}	
		
		char buf1[blksize] ;
		if(i<blknum)
		{	
			/*	Read the data remaining of that block	*/			
			readDataBlock(blkadr+i , &buf1);
			/*	Concate the read data with buffer data	*/
			strncat(buf1 , buf, blksize - offset); 
			//printf("new buffer = %s with length = %d\n" , buf1,strlen(buf1));
			/*	Write into the file(secondary memory)	*/
			writeDataBlock(blkadr+i , &buf1);
			i += 1;
			remSize = remSize - (blksize - offset);
			while(i<blknum && remSize>0)
			{
				/*	Write into the data block of secondary memory	*/
				writeDataBlock(blkadr+i ,(char *)&buf[i*blksize]);
				/*//printf("remSize > 0 \n");
				if(remSize > blksize)
				{
					writeBlock(blkadr+i , &buf[i*blksize] , blksize , offset);
				}	
				else
				{
					writeDataBlock(blkadr+i ,&buf[i*blksize]);
					writeBlock(blkadr+i , &buf[i*blksize] , remSize , offset);
				}*/
				++i;
				remSize = remSize - blksize;
			}	
			/*	If the data written is less than maximum size of the file	*/	
			if(i < blknum)	
				updateInodeFileSize(iNum , inodesize+size);
			/*	If the data written exceeds the maximum size of the file	*/
			else
				updateInodeFileSize(iNum ,inodesize+blksize * i);
			/*	Update Superblock and inode data	*/
			WriteFS();
			return size;
		}
		return 0;
	}
	
}


static int do_read(const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{
	printf("Read Called with size %ld , offset %ld\n",size, offset);
	
	if(offset == 0)
	{
		int iNum = getInodeNum(path);
		/*	Check if the file exists	*/
		if(iNum == -1)
			return -ENOENT;
		int blkadr = getInodeBlkAdr(iNum);
		int blknum = getInodeBlkNum(iNum);
		int blksize = getBlockSize();
		int fileSize = getInodeFileSize(iNum);
		int i = 0;
		int remSize ;
		if(fileSize > size)
			remSize = size;
		else
			remSize = fileSize;
					
		while(remSize > 0 && i < blknum)
		{
			/*	Read data from the block and store it into buffer	*/
			readDataBlock(blkadr+i , (char *) &buf[i*blksize] );	
			/*if(remSize > blksize)
			{
				//printf("Read Data from dgmfs = %s \n",	buf[i*blksize]);	
				//readBlock(blkadr+i , &buf[i*blksize] , blksize , offset);
			}	
			else
			{
				readDataBlock(blkadr+i ,  &buf[i*blksize] );			
				//printf("Read Data from dgmfs = %s \n",	buf[i*blksize]);	

				//readBlock(blkadr+i , &buf[i*blksize] , remSize , offset);				
			}*/
			++i;
			remSize = remSize - blksize;
		}
	
		/*	Return the amount of data read	*/
		if(i < blknum)
			return size;
		else
			return getInodeFileSize(iNum);
	}
	else
	{
		/*	Function not implemented	*/
		return -ENOSYS;
	}
}

// static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
// {
// 	printf( "--> Trying to read %s, %u, %u\n", path, offset, size );
// 	int inodeNum  = getInodeNum(path);
// 	int dbaddr = getInodeBlkAdr(inodeNum);
// 	char *selectedText = NULL;
// 	printf("data is %s\n", fs.dblock[dbaddr].data );
// 	selectedText = fs.dblock[dbaddr].data;
// 	memcpy( buffer, selectedText + offset, size );
// 	update_times(inodeNum, U_ATIME);
// 	return strlen( selectedText ) - offset;
// }
static int do_unlink(const char *path)
{
	printf("Unlink called \n");
	int parentInode = getParentInodeNum(path);
	int blkadr = getInodeBlkAdr(parentInode);
	//printf("blk address = %d",blkadr);
	char fname[MAX_FILE_LENGTH];
	int size = getDirSize(blkadr);
	int i;
	for(i=0 ; i<size ;++i)
	{
		getFileName(path,fname);
		if(strcmp(fs.dblock[blkadr].dir.dent[i].f_name,fname)==0)
		{
			if(i != size-1)
			{
				strcpy(fs.dblock[blkadr].dir.dent[i].f_name,fs.dblock[blkadr].dir.dent[size-1].f_name);
				fs.dblock[blkadr].dir.dent[i].st_ino = fs.dblock[blkadr].dir.dent[size-1].st_ino;
			}
			//fs.dblock[blkadr].dir.size = fs.dblock[blkadr].dir.size - 1 ; 
			updateDirSize(blkadr , size-1);	
			decLink(parentInode);
			return 0;
		}
	}
	WriteFS();
	if(i == size)
		return -ENOENT;

	return 0;
}


static int do_rmdir(const char *path)
{
	printf("Remove directory called \n");
	/*	Get parent inode number		*/
	int parentInode = getParentInodeNum(path);
	int blkadr = getInodeBlkAdr(parentInode);
	char fname[MAX_FILE_LENGTH];
	/*	Get only file name from the path	*/
	getFileName(path,fname);
	int iNum = getInodeNumFile(parentInode , fname);
	//printf("iNum = %d for path = %s \n",iNum,path);
	int size = getDirSize(blkadr);
	int i;
	for(i=0 ; i<size ;++i)
	{
		/*	Check if the directory entry exists 	*/
		if(strcmp(fs.dblock[blkadr].dir.dent[i].f_name,fname)==0)
		{
			int cblkadr = getInodeBlkAdr(iNum);
			int csize = getDirSize(cblkadr);
			//printf("csize = %d ,blkadr = %d \n",csize,cblkadr);
			/*	If the directory is empty (contains only "." and ".." in its directory entry file)	*/
			if(csize == 2)
			{
				/*	Check if this is the last entry of the file	*/
				if(i != size-1)
				{
					strcpy(fs.dblock[blkadr].dir.dent[i].f_name,fs.dblock[blkadr].dir.dent[size-1].f_name);
					fs.dblock[blkadr].dir.dent[i].st_ino = fs.dblock[blkadr].dir.dent[size-1].st_ino;
				}
				//fs.dblock[blkadr].dir.size = fs.dblock[blkadr].dir.size - 1 ; 
				/*	Update the directory index 	*/	
				updateDirSize(blkadr , size-1);
				/*	Decrement the number of links of the parent 	*/ 
				decLink(parentInode);
				return 0;
			}
			/*	If the directory is not empty	*/
			else
			{
				return -ENOTEMPTY;
			}
		}
	}
	/*	Update superblock and inode data	*/
	WriteFS();

	if(i == size)
		return -ENOENT;
	return 0;	
}

// int do_rmdir(const char *path)
// {
// 	int parentInode = getParentInodeNum(path);
// 	int dbaddr = getInodeBlkAdr(parentInode);
// 	int index = getIndexOfFile(path);
// 	fs.dblock[dbaddr].dir.dent[index].st_ino = -1;
// 	strcpy(fs.dblock[dbaddr].dir.dent[index].f_name, "-----");
// 	decLink(parentInode);
// 	return 0;
// }
// static int do_mknod(const char *path, mode_t mode)
// {
// 	int inodeNum  = getInodeNum(path);
// 	printf("came to mknode\n");
// 	if (inodeNum == -1)
// 	{
// 		int inodeNum = createInode(path , mode , Regular);
// 		addToDir(path, inodeNum);

// 	}
// 	return 0;
// }


static struct fuse_operations operations = {
	.getattr = do_getattr,
	.mkdir   = do_mkdir,
	.readdir = do_readdir,
	.create = do_create,
	.read    = do_read,
	// .mknod   = do_mknod,
	.unlink = do_unlink,
	.write   = do_write,
	.rmdir   = do_rmdir,
	.open    = do_open,
	// .utimens = do_utimens,

};


int main(int argc , char *argv[])
{
	if (makefs() != -1)
	{
		struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
		return fuse_main(args.argc, args.argv, &operations, NULL);
	}
}