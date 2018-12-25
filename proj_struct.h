#define BLOCKS_PER_INODE 6		//For each file, number of blocks allocated
#define BLOCK_SIZE 2048
#define MAX_FILE_LENGTH 30
#define MAX_FILES 30

#define TOTAL_DATA_BLOCKS MAX_FILES * BLOCKS_PER_INODE
#define TOTAL_INODE_BLOCKS MAX_FILES

#include <stdint.h>

typedef uint32_t UINT; //This type must be 4 bytes

#define mode_t int

enum filetype {Regular = 0 , Directory = 1};
#define type_t enum filetype

struct inode_stat
{
	int    st_ino;			/* Inode number */
	mode_t    st_mode;		/*Mode of Permission*/
	type_t    st_type;		/*Type of the file*/
	int    st_nlink;			/* Number of hard links */
	int    st_uid;			/* User ID of owner */
	int    st_gid;			/* Group ID of owner */
	int    st_size;			/* Total size, in bytes */
	int    st_blkadr;        	/*Starting Block Number of the file*/
	int    st_blknum	;		/*Number of Current Blocks*/
	// time_t st_atim;
	// time_t st_ctim;
	// time_t st_mtim;
};

//Super Block Structure
struct super_block
{
	UINT block_size;		//Size of the block
	UINT num_blocks;		//Total number of blocks in memory
	UINT num_inode;
	UINT num_free_blocks;	//Number of free blocks in memory
	UINT num_free_inode;	//Number of free inodes in memory
	UINT free_block_head;	//Points to the first free block(Block Number)
	UINT root_ino;		//Root inode number
//	struct inode_stat* current_inode ;	//Currently free inode address
	UINT current_inode_no;	//Currently free inode number
};

//Directory Entry Structure
struct dentry
{
	int st_ino;	//Inode Pointer of the file
	char f_name[MAX_FILE_LENGTH];	//File path
};

struct direct
{
	struct dentry dent[MAX_FILES];
	int size;
};

union data_block
{
	char * data;
	struct direct dir;
};
