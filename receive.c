#include "protocol/protocolAPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include "protocol/serialProtocol.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int openFile(char*filename){
	return open(filename,O_WRONLY);
}

unsigned char* extractDataFromPacket(unsigned char* data,unsigned int* sizeOfData){
	unsigned char* buffer = (unsigned char*)malloc(*sizeOfData-4);
	if(buffer==NULL){
		printf("[ERROR]\tallocation error\n");
		return NULL;
	}else{
		memcpy(buffer,&data[4],*sizeOfData-4);
		*sizeOfData = *sizeOfData-4;
		return buffer;
	}
}

int main()
{
	char filename[255];
	int file;
	char *buf = NULL;
	int fd = llopen(0, RECEIVER);
	int rd;
	while(buf==NULL || strcmp(buf,"DC")){
		rd = llread(fd, &buf);
		if(rd==-1){
			printf("unable to read\n");
			int ret = llclose(fd);
			if (ret == 1)
			{
				printf("closed\n");
			}
			return -1;
		}else{
			if(buf[0]==1){
				//data packet
				unsigned int sz = rd;
				unsigned char*buffer =  extractDataFromPacket(buf,&sz);
				unsigned int rez = write(file,buffer,sz);
				free(buffer);
				if(rez!=sz){
					strcpy(buf,"DC");
				}
			}
			if(buf[0]==2){
				//start packet
				strcpy(filename,&buf[3]);
				file = openConnection(&filename);
				if(file<1){
					strcpy(buf,"DC");
				}
			}
			if(buf[0]==3){
				//end packet
				strcpy(buf,"DC");
			}
		}
	}
	if (!strcmp("DC", buf))
	{
		close(file);
		int ret = llclose(fd);
		if (ret == 1)
		{
			printf("closed\n");
		}
	}
	return 0;
}