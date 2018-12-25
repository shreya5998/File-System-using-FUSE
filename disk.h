#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef	BLOCK_SIZE
#define BLOCK_SIZE 2048
#endif

int OpenDisk(char *filename , int nbytes);
int ReadBlock(int disk , int blocknr , void *block);
int WriteBlock(int disk , int blocknr , void *block);
