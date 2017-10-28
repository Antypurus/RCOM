#include "protocol/protocolAPI.h"
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include "protocol/serialProtocol.h"

int main()
{
	char *buf = NULL;
	int fd = llopen(0, RECEIVER);
	int rd = llread(fd, buf);
	if (rd == 0)
	{
		printf("unable to read\n");
		int ret = llclose(fd);
		if (ret == 1)
		{
			printf("closed\n");
		}
		return -1;
	}
	if (!strcmp("DC", buf))
	{
		int ret = llclose(fd);
		if (ret == 1)
		{
			printf("closed\n");
		}
	}
	return 0;
}