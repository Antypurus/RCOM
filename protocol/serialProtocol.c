#include<serialProtocol.h>
#include<Math.h>
#include<stdlib.h>

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