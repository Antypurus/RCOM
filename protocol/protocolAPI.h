#pragma once

struct sideControll{
    int side;
};

struct sideControll g_side;

#define TRANSMITER 0
#define RECEIVER 1

//NEEDS DOCUMENTATION
int llopen(int porta,int side);//llopen

//NEEDS DOCUMENTATION
int llread(int fd,char** buffer);//llread

//NEEDS DOCUMENTATION
int llwrite(int fd,char*buffer,int lenght);//llwrite

//NEEDS DOCUMENTATION
int llclose(int fd);//llclose