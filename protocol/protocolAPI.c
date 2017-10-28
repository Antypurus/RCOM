#include "serialProtocol.h"
#include "protocolAPI.h"

int llopen(int porta, int side)
{
    unsigned char port[20];
    if (porta == 0)
    {
        port = "dev/ttyS0";
    }
    else
    {
        port = "dev/ttyS1";
    }

    unsigned int fd = openConnection(port, 0);

    if (side == RECEIVER)
    {
        g_side.side = RECEIVER;
        unsigned int sz = 0;
        unsigned char *data = readSentData(fd, &sz);
        if (data == NULL)
        {
            printf("SET FAILED !!!\n");
            exit(-1);
        }
        else
        {
            if (sz != 5)
            {
                printf("Frame is not a command frame due to incoherent size!!!\n");
                return -1;
            }
            else
            {
                unsigned char ret = validateFrame(data, sz);
                if (ret == 1 && g_ctrl.shouldDC == FALSE)
                {
                    ret = sendUACommand(fd);
                    if(ret==0){
                        printf("unable to send UA command\n");
                        return -1;
                    }
                    printf("SET frame read and UA command sent!!!\n");
                    return (int)fd;
                }else{
                    printf("this is not a set frame error!!!\n");
                    return -1;
                }
            }
        }
    }
    else
    {
        g_side.side = TRANSMITER;
        unsigned char ret = sendSetCommand(fd);
        if(ret==0){
            printf("failed to send set command\n");
            return -1;
        }else{
            printf("set command send and valid response received");
            return (int)fd;
        }
    }

    return -1;
}

int llread(int fd, char *buffer)
{
}

int llwrite(int fd, char *buffer, int lenght)
{
}

int llclose(int fd)
{
}