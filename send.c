#include"protocol/protocolAPI.h"
#include<stdio.h>
#include <termios.h>
#include <unistd.h>
#include"protocol/serialProtocol.h"

int main(){
	int fd = llopen(0,TRANSMITER);
	sleep(1);
	llclose(fd);
	return 0;
}