#include<serialProtocol.h>
#include<Math.h>

unsigned int allocateInformationFrames(unsigned char** buff,const unsigned char data[]){
    unsigned int size = (unsigned int)floor(sizeof(data)/MAX_DATA_PER_FRAME);
}