#include "include/fileop.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

static int remove_content(char *filename)
{
	FILE *fp = fopen(filename, "w+");
	if(fp != NULL)
	{
		fclose(fp);
		return 0;
    	}
	return -1;
	
}

int init_open_file(char *filename)
{
	int fd;
	if(filename == NULL)
	{
		return -1;
	}
	if(remove_content(filename) == -1)
	{
		return -1;
	}
	if((fd = open(filename, O_RDONLY)) < 0)
	{
		return fd;
	}
	return fd;
}
