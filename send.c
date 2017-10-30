#include "protocol/protocolAPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "protocol/serialProtocol.h"

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
		fi(buffer == NULL){
			printf("[ERROR]\tallocation error\n");
			return NULL;
		}else{
			buffer[0] = 1;
			buffer[1] = (g_seq++)%255;
			buffer[2] = 0;
			buffer[3] = sizeOfData;
			memcpy(&buffer[4],data,sizeOfData);
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
		if (buffer == NULL)
		{
			printf("[ERROR]\tallocation error\n");
			return NULL;
		}
		else
		{
			buffer[0] = startEnd;
			buffer[1] = 1;
			buffer[2] = filenameSize;
			memcpy(&buffer[3], filename, filenameSize);
			return buffer;
		}
	}
}

int openFile(char* filename){
	return open(filename,O_RDONLY | O_NONBLOCK );
}

int main()
{
	char noStuff[] = "Hello My Good Friend";
	char stuff[4];
	stuff[0] = 0x65;
	stuff[1] = 0x7E;
	stuff[2] = 0x7D;
	stuff[3] = 0x65;

	int fd = llopen(0, TRANSMITER);
	printf("Sending %s\n", noStuff);
	int read = llwrite(fd, noStuff, sizeof(noStuff));
	printf("Wrote %d\n", read);

	printf("Sending %s\n", stuff);
	read = llwrite(fd, stuff, sizeof(stuff));
	printf("Wrote %d\n", read);

	llclose(fd);
	return 0;
}