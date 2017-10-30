#include "protocol/protocolAPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "protocol/serialProtocol.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define maxBytes 99

unsigned int g_seq = 0;

unsigned char **createDataPacket(unsigned char *data, unsigned int sizeOfData, unsigned int *sizeOfPacket)
{
	unsigned int maxDataSize = maxBytes - 4;
	if (sizeOfData > maxDataSize)
	{
		printf("[ERROR]\tDataset too large\n");
		return NULL;
	}
	else
	{
		unsigned int needed = 4 + sizeOfData;
		*sizeOfPacket = needed;
		unsigned char *buffer = (unsigned char)malloc(needed);
		if (buffer == NULL)
		{
			printf("[ERROR]\tallocation error\n");
			return NULL;
		}
		else
		{
			buffer[0] = 1;
			buffer[1] = (g_seq++) % 255;
			buffer[2] = 0;
			buffer[3] = sizeOfData;
			memcpy(&buffer[4], data, sizeOfData);
			return buffer;
		}
	}
}

unsigned char *createControllPacket(unsigned char startEnd, char *filename, unsigned char filenameSize, unsigned int filesize, unsigned int *sizeOfPacket)
{
	*sizeOfPacket = 3 + filenameSize; //ignoring the filesize
	if (*sizeOfPacket > maxBytes)
	{
		printf("[ERROR]\tProtocol does not support a single packet this large, max is 99 bytes\n");
		return NULL;
	}
	else
	{
		unsigned char *buffer = (unsigned char)malloc(*sizeOfPacket);
		printf("\n\nhere\n");
		if (buffer == NULL)
		{
			printf("[ERROR]\tallocation error\n");
			return NULL;
		}
		else
		{
			printf("here\n");
			buffer[0] = startEnd;
			buffer[1] = 1;
			buffer[2] = filenameSize;
			memcpy(&buffer[3], filename, filenameSize - 1);
			return buffer;
		}
	}
}

int openFile(char *filename)
{
	return open(filename, O_RDONLY | O_NONBLOCK);
}

int main()
{
	char filename[] = "protocol/pinguim.gif";
	char buff[255];
	int file = openFile(&filename);
	if (file < 0)
	{
		return -1;
	}
	int fd = llopen(0, TRANSMITER);
	unsigned int sz = 0;
	unsigned char *buffer = createControllPacket(2, filename, strlen(filename) + 1, 0, &sz);
	printf("finished creating controll packet\n");
	if (buffer == NULL)
	{
		printf("here\n");
		llclose(fd);
		return -1;
	}
	printf("here\n");
	unsigned int res = llwrite(file, buffer, sz);
	if (res != sz)
	{
		llclose(fd);
		return -1;
	}
	printf("here\n");
	free(buffer);
	res = 255;
	while (res != 0)
	{
		printf("here\n");
		res = read(fd, &buff, maxBytes - 4);
		buffer = createDataPacket(buff, res, &sz);
		if (buffer == NULL)
		{
			llclose(fd);
			return -1;
		}
		sz = llwrite(fd, buff, sz);
		if (sz == 0)
		{
			llclose(fd);
			return -1;
		}
	}
	buffer = createControllPacket(3, filename, strlen(filename) + 1, 0, &sz);
	if (buffer == NULL)
	{
		llclose(fd);
		return -1;
	}
	res = llwrite(file, buffer, sz);
	if (res != sz)
	{
		llclose(fd);
		return -1;
	}
	free(buffer);

	llclose(fd);
	return 0;
}