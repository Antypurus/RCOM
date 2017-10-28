#include"protocolAPI.h"
#include<stdio.h>

int main(){
	int fd = llopen(0,TRANSMISTER);
	slee(1);
	llclose(fd);
	return 0;
}