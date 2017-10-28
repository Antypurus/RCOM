#include"protocolAPI.h"
#include<stdio.h>
#include<string.h>

int main(){
	char* buf;
	int fd = llopen(0,TRANSMISTER);
	int rd = llread(fd,buf);
	if(!strcmp("DC",buf)){
		int ret = llclose(fd);
		if(ret==1){
			printf("closed\n");
		}
	}
	return 0;
}