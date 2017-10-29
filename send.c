#include"protocol/protocolAPI.h"
#include<stdio.h>
#include <termios.h>
#include <unistd.h>
#include"protocol/serialProtocol.h"

int main(){
	char noStuff[] = "Hello My Good Friend";
	char stuff[4];
	stuff[0]=0x65;
	stuff[1]=0x7E;
	stuff[2]=0x7D;
	stuff[3]=0x65;

	int fd = llopen(0,TRANSMITER);
	printf("Sending %s\n",noStuff);
	int read = llwrite(fd,noStuff,sizeof(noStuff));
	printf("Wrote %d\n",read);

	printf("Sending %s\n",stuff);
	read = llwrite(fd,stuff,sizeof(stuff));
	printf("Wrote %d\n",read);

	llclose(fd);
	return 0;
}