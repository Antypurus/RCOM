#include <serialProtocol.h>
#include <Math.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#define NULL 0

unsigned int allocateInformationFrames(unsigned char** buff,const unsigned char data[]){
    //This bit of code determine how many frames will be necessary to send the data
    unsigned int size = (unsigned int)floor(sizeof(data)/MAX_DATA_PER_FRAME);

    //this allocates the buffer with the ammount of frames necessary to send the data
    buff = (unsigned char**)malloc(sizeof(unsigned char*)*size);

    if(buff == NULL){
        deallocateInformationFrames(buff,0);//since the was an error everything must be dealocated
        return -1;// this indicates there was an issue allocating the requires space
    }

    //this allocates the space required to send the data inside each frame
    for(unsigned int i=0;i<size:++i){
        buff[i] = (unsigned char*)malloc(sizeof(unsigned char)*MAX_FRAME_SIZE);

        if(buff[i]==NULL){
            deallocateInformationFrames(buff,i);//since the was an error everything must be dealocated
            return -1;// this indicates there was an issue allocating the requires space
        }

    }
    return size;//returns the ammount of frames allocated
}

void deallocateInformationFrames(unsigned char** frames,unsigned int numberOfFrames){
    //dealocates the memory space of each individual frame
    for(unsigned int i=0;i<numberOfFrames:++i){
        free(frames[i]);
    }
    //dealocates the frame buffer
    free(frames);
    return;
}

unsigned char** allocateCharBuffers(unsigned int numberOfBuffers,unsigned int dataPerBuffer){
    //allocates the buffer of buffers
    unsigned char** buffer = (unsigned char**)malloc(sizeof(unsigned char*)*numberOfBuffers);

    if(buffer = NULL){
        deallocatedCharBuffers(buffer,0);//deallocates the buffer of buffers at is not needed
        return -1;//indicates there has been an error with the allocation
    }

    //allocate each individual buffer
    for(unsigned int i=0;i<numberOfBuffers;++i){
        buffer[i] = (unsigned char*)malloc(sizeof(unsigned char)*dataPerBuffer);

        if(buffer[i]==NULL){
            deallocatedCharBuffers(buffer,0);//deallocates the buffer of buffers at is not needed
            return -1;//indicates there has been an error with the allocation
        }

    }

    return buffer;
}

void deallocatedCharBuffers(unsigned char** buffers,unsigned int numberOfBuffers){
    //dealocates the memory space of each buffer
    for(unsigned int i=0:i<numberOfFrames){
        free(buffers[i]);
    }
    //delocates the buffer of buffers
    free(buffers);
    return;
}

unsigned char** divideData(const unsigned char data[]){
    unsigned int size = (unsigned int)floor(sizeof(data)/MAX_DATA_PER_FRAME);//determine the size of the data
    unsigned char** buffer = allocateCharBuffers(size,MAX_DATA_PER_FRAME);//allocates the required space for the data holders

    if(buffer==-1){
        return -1;//return an error code, meaning there was an error with the allocation
    }

    //moves each chunk of data to the buffers
    for(unsigned int i=0;i<size;++i){
        memmove(buffer[i],data+i*MAX_DATA_PER_FRAME,MAX_DATA_PER_FRAME);
    }

    return buffer;
}

void moveInformationToFrame(unsigned char* frame,const unsigned char data[]){
    if(sizeof(data)>MAX_DATA_PER_FRAME){
        return; //The data is too big for this frame;
    }

    memmove(frame+4,data,sizeof(data));//moves the data to the data section of the frame
    return;
}

void prepareInformationFrames(unsigned char** frames,unsigned int numberFrames){
    for(unsigned int i=0;i<numberFrames;++i){
        frames[i][0]=FLAG;//FLAG
        frames[i][1]=ADDRS;//ADDRS
        frames[i][2]=((i%2)<<5);//CTRL
        frames[i][3]=frames[i][1] ^ frames[i][2];//BCC1
        frames[i][MAX_FRAME_SIZE-1]=FLAG;//FLAG
    }
    return;
}

void moveDataToFrames(unsigned char** frames,const unsigned char data[],unsigned int numberOfFrames){
    unsigned char info[][] = divideData(data);//obtain the data divided into chunks

    for(unsigned int i=0;i<numberFrames;++i){
        moveInformationToFrame(frames[i],info[i]);//moves the chunk of data into the frame
        frames[i][MAX_FRAME_SIZE-2] = calculateBCC2(info[i]);//sets the BCC for the data chunk that was moved
    }

    return;
}

unsigned char calculateBCC2(const unsigned char data[]){
    unsigned char ret = 0;
    unsigned int size = sizeof(data)/sizeof(unsigned char);//determine the size of the data

    for(unsigned int i=0;i<size;++i){
        ret = ret ^ data[i];//calculate the XOR of the whole data
    }

    return ret;
}