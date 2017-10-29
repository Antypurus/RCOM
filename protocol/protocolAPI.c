#include "serialProtocol.h"
#include "protocolAPI.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

int llopen(int porta, int side)
{
    char port[20];
    if (porta == 0)
    {
        memcpy(port, "/dev/ttyS0", 11);
    }
    else
    {
        memcpy(port, "/dev/ttyS0", 11);
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
                    if (ret == 0)
                    {
                        printf("unable to send UA command\n");
                        return -1;
                    }
                    printf("SET frame read and UA command sent!!!\n");
                    return (int)fd;
                }
                else
                {
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
        if (ret == 0)
        {
            printf("failed to send set command\n");
            return (int)fd;
        }
        else
        {
            printf("set command send and valid response received");
            return (int)fd;
        }
    }

    return -1;
}

int llread(int fd, char **buffer)
{
    unsigned int size = 0;
    unsigned char *data;
jmp:
    data = readSentData(fd, &size);
    if (data == NULL)
    {
        printf("failed to read data\n");
    }
    else
    {
        unsigned char ret = validateFrame(data, size);
        if (ret == 1)
        {
            if (g_ctrl.shouldDC)
            {
                ret = sendUACommand(fd);
                if (ret == 0)
                {
                    printf("failed to send UA command\n");
                }
                *buffer = (char *)malloc(3); //allocated space for the string DC
                if (buffer == NULL)
                {
                    printf("failed to allocated buffer\n");
                    return -1;
                }
                *buffer = "DC";
                return 3; //received a disconnect
            }
            else
            {
                printf("read out if place SET command , ignoring\n");
                goto jmp;
            }
        }
        else
        {
            if (ret == 2)
            { //received a data frame
                ret = sendRRCommand(fd);
                if (ret == 0)
                {
                    printf("failed to send RR command\n");
                    return -1;
                }
                extractDataFromFrame(data, &size);
                if (data == NULL)
                {
                    printf("unable to extract data from frame\n");
                    return -1;
                }
                else
                {
                    destuffData(data, &size);
                    if (data == NULL)
                    {
                        printf("unable to destuff data\n");
                        return -1;
                    }
                    else
                    {
                        *buffer = data;
                        return size;
                    }
                }
            }
            else
            {
                ret = sendREJCommand(fd);
                if (ret == 0)
                {
                    printf("failed to send rej command, CRITICAL ERROR!\n ");
                    llclose(fd);
                    exit(-1);
                }
                return -1;
            }
        }
    }
    return -1;
}

int llwrite(int fd, char *buffer, int lenght)
{
    unsigned int ret = sendData(fd, buffer, lenght);
    if (ret != lenght)
    {
        printf("failed to send data or validate response\n");
        return -1;
    }
    else
    {
        return ret;
    }
    return -1;
}

int llclose(int fd)
{
    if (g_side.side == RECEIVER)
    {
        closeConnection(fd);
        return 1;
    }
    else
    {
        unsigned int ret = sendDisconnectCommand(fd);
        if (ret == 0)
        {
            printf("failed to send disconnect command\n");
            closeConnection(fd);
            return -1;
        }
        closeConnection(fd);
        return 1;
    }
    return 0;
}