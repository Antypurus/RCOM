#include "serialProtocol.h"
#include <math.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

unsigned int allocateInformationFrames(unsigned char** buff,const unsigned char data[],unsigned int sizeOf){
    printf("[LOG]@memory\tStarting Allocation of information frames process\n");
    //This bit of code determine how many frames will be necessary to send the data
    unsigned int size = (unsigned int)floor(sizeOf/MAX_DATA_PER_FRAME);
    
    //this allocates the buffer with the ammount of frames necessary to send the data
    buff = (unsigned char**)malloc(sizeof(unsigned char*)*size);
    
    if(buff == NULL){
        printf("[ERROR]@memory\tAllocation of the information frame failed\n");
        deallocateInformationFrames(buff,0);//since the was an error everything must be dealocated
        return -1;// this indicates there was an issue allocating the requires space
    }

    printf("[LOG]@memory\tAllocated information frame buffer of size %d\n",size);

    //this allocates the space required to send the data inside each frame
    for(unsigned int i=0;i<size;++i){
        if(size>1){
            if(i == (size -1)){
                double fraction = size - (sizeOf/MAX_DATA_PER_FRAME);//determines fraction of the max data per frame for the last frame
                g_ctrl.lastFrameSize = sizeof(unsigned char)*MAX_FRAME_SIZE*fraction;//saves the size of the last frame for easy acess
                buff[i] = (unsigned char*)malloc(sizeof(unsigned char)*MAX_FRAME_SIZE*fraction);//allocates the frame with only the needed size
                printf("[LOG]@memory\tAttempting to allocate an information frame of %d bytes",MAX_FRAME_SIZE*fraction);
            }else{
                buff[i] = (unsigned char*)malloc(sizeof(unsigned char)*MAX_FRAME_SIZE);
                printf("[LOG]@memory\tAttempting to allocate an information frame of %d bytes",MAX_FRAME_SIZE);
                
            }
        }else{
            buff[i] = (unsigned char*)malloc(sizeof(unsigned char)*(sizeOf+6));
            printf("[LOG]@memory\tAttempting to allocate an information frame of %d bytes",(sizeOf+6));
        }

        if(buff[i]==NULL){
            printf("[ERROR]@memory\tInformation Frame Failed to Allocate\n");
            deallocateInformationFrames(buff,i);//since the was an error everything must be dealocated
            return -1;// this indicates there was an issue allocating the requires space
        }

        printf("[SUCCES]@memory\tInformation Frame Successfully Allocated\n");
    }
    printf("[LOG]@memory\tAllocation of Information frames process ended\n");
    return size;//returns the ammount of frames allocated
}

void deallocateInformationFrames(unsigned char** frames,unsigned int numberOfFrames){
    printf("[LOG]@memory\tStarting deallocation of information frames\n");
    //dealocates the memory space of each individual frame
    for(unsigned int i=0;i<numberOfFrames;++i){
        free(frames[i]);
    }
    //dealocates the frame buffer
    free(frames);
    printf("[LOG]@memory\tFinished deallocating Information frames\n");
    return;
}

unsigned char** allocateCharBuffers(unsigned int numberOfBuffers,unsigned int dataPerBuffer){
    printf("[LOG]@memory\tStarting allocation of generic buffer\n");
    //allocates the buffer of buffers
    printf("[LOG]@memory\tAtempting to allocate buffer of size %d\n",numberOfBuffers);
    unsigned char** buffer = (unsigned char**)malloc(sizeof(unsigned char*)*numberOfBuffers);

    if(buffer == NULL){
        printf("[ERROR]@memory\tFailed to allocated buffer\n");
        deallocatedCharBuffers(buffer,0);//deallocates the buffer of buffers at is not needed
        return NULL;//indicates there has been an error with the allocation
    }
    printf("[SUCCESS]@memory\tBuffer holder allocated\n");

    //allocate each individual buffer
    for(unsigned int i=0;i<numberOfBuffers;++i){
        buffer[i] = (unsigned char*)malloc(sizeof(unsigned char)*dataPerBuffer);
        printf("[LOG]@memory\tAttempting to allocated generic buffer of size %d\n",dataPerBuffer);

        if(buffer[i]==NULL){
            printf("[ERROR]@memory\tFailed to allocated buffer\n");
            deallocatedCharBuffers(buffer,0);//deallocates the buffer of buffers at is not needed
            return NULL;//indicates there has been an error with the allocation
        }
        printf("[SUCCESS]@memory\tBuffer allocated\n");
    }
    printf("[LOG]@memory\tgeneric buffer allocation finished\n");
    return buffer;
}

void deallocatedCharBuffers(unsigned char** buffers,unsigned int numberOfBuffers){
    printf("[LOG]@memory\tStarting generic buffer deallocation\n");
    //dealocates the memory space of each buffer
    for(unsigned int i=0;i<numberOfBuffers;++i){
        free(buffers[i]);
    }
    printf("[LOG]@memory\tgeneric buffers freed\n");
    //delocates the buffer of buffers
    free(buffers);
    printf("[LOG]@memory\tFinsished generic buffer deallocation\n");
    return;
}

unsigned char** divideData(const unsigned char data[],unsigned int* sizeOf){
    unsigned int size = (unsigned int)floor(*sizeOf/MAX_DATA_PER_FRAME);//determine the size of the data
    unsigned char** buffer = allocateCharBuffers(size,MAX_DATA_PER_FRAME);//allocates the required space for the data holders

    if(buffer==NULL){
        return NULL;//return an error code, meaning there was an error with the allocation
    }

    //moves each chunk of data to the buffers
    for(unsigned int i=0;i<size;++i){
        if((i*MAX_DATA_PER_FRAME+MAX_DATA_PER_FRAME)>*sizeOf){
            unsigned int str = i*MAX_DATA_PER_FRAME;//calculate the start of the range
            unsigned int end = *sizeOf-i*MAX_DATA_PER_FRAME;//calculate how much to copy at the end of the range
            memmove(buffer[i],data+str,end);
            *sizeOf = *sizeOf-i*MAX_DATA_PER_FRAME;
        }else{
            memmove(buffer[i],data+i*MAX_DATA_PER_FRAME,MAX_DATA_PER_FRAME);
        }
    }

    return buffer;
}

void moveInformationToFrame(unsigned char* frame,const unsigned char data[],unsigned int size){
    if(size>MAX_DATA_PER_FRAME){
        return; //The data is too big for this frame;
    }

    memmove(frame+4,data,size);//moves the data to the data section of the frame
    return;
}

void prepareInformationFrames(unsigned char** frames,unsigned int numberFrames){
    for(unsigned int i=0;i<numberFrames;++i){
        
        if(g_ctrl.currPar==0){
            g_ctrl.currPar = 1;
        }else{
            g_ctrl.currPar = 0;
        }

        unsigned int lastByte = 0;
        if(i == numberFrames-1){
            lastByte = g_ctrl.lastFrameSize;
        }else{
            lastByte = MAX_FRAME_SIZE;
        }

        frames[i][0]=FLAG;//FLAG
        frames[i][1]=ADDRS;//ADDRS
        frames[i][2]=g_ctrl.currPar;//CTRL
        frames[i][3]=frames[i][1] ^ frames[i][2];//BCC1
        frames[i][lastByte-1]=FLAG;//FLAG
    }
    return;
}

void moveDataToFrames(unsigned char** frames,const unsigned char data[],unsigned int size,unsigned int numberOfFrames){
    unsigned int s_size = size;
    unsigned char** info = divideData(data,&s_size);//obtain the data divided into chunks

    for(unsigned int i=0;i<numberOfFrames;++i){
        unsigned int sizeOf = 0;//size of the current frame
        if(i==numberOfFrames-1){
            sizeOf = g_ctrl.lastFrameSize;
        }else{
            sizeOf = MAX_FRAME_SIZE;
        }
        unsigned char*stuffed = byteStuffingOnData(info[i],&s_size);
        if(stuffed == NULL){
            g_ctrl.allocError = 1;
            return;
        }
        g_ctrl.allocError = 0;
        moveInformationToFrame(frames[i],stuffed,s_size);//moves the chunk of data into the frame
        frames[i][sizeOf-2] = calculateBCC2(info[i],s_size);//sets the BCC for the data chunk that was moved
    }

    return;
}

unsigned char calculateBCC2(const unsigned char data[],unsigned int sizeOfData){
    unsigned char ret = 0;
    unsigned int size = sizeOfData/sizeof(unsigned char);//determine the size of the data

    for(unsigned int i=0;i<size;++i){
        ret = ret ^ data[i];//calculate the XOR of the whole data
    }

    return ret;
}

//Need to document and properly adapt the code
unsigned int openConnection(char* serialPort,unsigned int flags){
    
    if ( ((strcmp("/dev/ttyS0", serialPort)!=0) && (strcmp("/dev/ttyS1", serialPort)!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    unsigned int fd;
    fd = open(serialPort, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd <0) {perror(serialPort); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    return fd;
}

void closeConnection(unsigned int fd){
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
      }
  
    close(fd);
}

//NEED TO DOCUMENT - MISSING ERROR CHECKING
unsigned char* byteStuffingOnData(const unsigned char data[],unsigned int* sizeOfData){
    unsigned int originalSize = *sizeOfData;
    unsigned int postSize = originalSize;

    unsigned char** buffer = allocateCharBuffers(1,originalSize*2);// we really should not use this but as a barebones implementation it can pass

    unsigned int currIndice = 0;
    for(unsigned int i=0;i<originalSize;++i){
        if(data[i]==0x7E || data[i]==0x7D){
            buffer[0][currIndice] = 0x7D;
            buffer[0][++currIndice] = 0x5D;
            postSize++;
        }else{
            buffer[0][currIndice] = data[i];
        }
        currIndice++;
    }

    void* ret = realloc(buffer[0],postSize);
    if(ret==NULL){
        return NULL;
    }
    *sizeOfData = postSize;
    return buffer[0];
}

//NEEDS TO BE DOCUMENTED
char ReceptorResponseInterpreter(const unsigned char* receptorResponse){
    unsigned int currntState = FLAG_STR;
    unsigned int responseType;

    for(unsigned int i=0;i<5;++i){
        switch(currntState){

            case(FLAG_STR):{
                if(receptorResponse[i]==FLAG){
                    currntState = ADDR;
                }else{
                    return ERR;
                }
                break;
            }

            case(ADDR):{
                if(receptorResponse[i]==ADDR2){
                    currntState = CTRL;
                }else{
                    return ERR;
                }
                break;
            }

            case(CTRL):{
                if(g_ctrl.currPar==0){
                    if(receptorResponse[i]==UA){
                        responseType = UA_R;
                        currntState = BCC;
                        break;
                    }else if(receptorResponse[i]==RR0){
                        responseType = ACPT;
                        currntState = BCC;
                        break;
                    }else if(receptorResponse[i]==REJ0){
                        responseType = REJ;
                        currntState = BCC;
                        break;
                    }else{
                        return ERR;
                    }
                }else if(g_ctrl.currPar==1){
                    if(receptorResponse[i]==UA){
                        responseType = UA_R;
                        currntState = BCC;
                        break;
                    }else if(receptorResponse[i]==RR1){
                        responseType = ACPT;
                        currntState = BCC;
                        break;
                    }else if(receptorResponse[i]==REJ1){
                        responseType = REJ;
                        currntState = BCC;
                        break;
                    }else{
                        return ERR;
                    }
                }
                break;
            }

            case(BCC):{
                if(receptorResponse[i]==(receptorResponse[i-1]^receptorResponse[i-2])){
                    currntState = FLAG_END;
                    break;
                }else{
                    return ERR;
                }
            }

            case(FLAG_END):{
                if(receptorResponse[i]==FLAG){
                    currntState = DONE_PROC;                                      
                }else{
                    return ERR;
                }
            }

            default:break;
        }
    }

    if( currntState == DONE_PROC){
        return responseType;
    }

    return ERR;//error code
}

//NEEDS TO BE COMMENTED
unsigned char* getReceptorResponse(unsigned int fd){
    unsigned int res = 0;
    unsigned char* buffer = (unsigned char*)malloc(sizeof(unsigned char)*5);
    if(buffer == NULL){
        return NULL;
    }
    alarm(TIMEOUT);//sets an alarm with the timeout value , to manage connection timeouts
    while(res==0){
        res = read(fd,buffer,5);
        if(res==5){// if it reads it disables the alarms and marks the connection as successful and returns 1
            alarm(0);
            g_ctrl.hasTimedOut = FALSE;
            g_ctrl.retryCounter = 0;
            return buffer;
        }
        if(g_ctrl.hasTimedOut){// if the connection is marked as timed out 0 is returned
            return NULL;
        }
    }
    return NULL;
}

//NEEDS TO BE COMMENTED
unsigned char sendSetCommand(unsigned int fd){
    signal(SIGALRM,timeoutHandler);
    g_ctrl.currPar = 0;
    g_ctrl.retryCounter = 0;
    g_ctrl.fileDescriptor = fd;

    //configures a buffer with the information for the set command
    unsigned char buffer[5];
    buffer[0] = FLAG;
    buffer[1] = ADDR;
    buffer[2] = SET;
    buffer[3] = buffer[1] ^ buffer[2];
    buffer[4] = FLAG;

    g_ctrl.frameToSend = buffer;

    unsigned int res = write(fd,buffer,5);//writes to pipe the set command in the buffer

    unsigned char* buff;

    if(res==5){//if it wrote the whole buffer it will now atemp to read the confirmation for the receptor
        buff = getReceptorResponse(fd);//get the response with the possibility of timeout
        if(buff==NULL){
            return 0;
        }
        unsigned int rez = ReceptorResponseInterpreter(buff);//check response type
        if(rez == UA_R){//if correct response type return 1
	    free(buff);
            return 1;
        }else{
	    free(buff);
            return 0;
        }
    }else{
        return 0;
    }
    return 0;
}

unsigned char sendDisconnectCommand(unsigned int fd){
    signal(SIGALRM,timeoutHandler);
    g_ctrl.retryCounter = 0;
    g_ctrl.fileDescriptor = fd;

    unsigned char buffer[5];
    buffer[0] = FLAG;
    buffer[1] = ADDR;
    buffer[2] = DISC;
    buffer[3] = buffer[1] ^ buffer[2];
    buffer[4] = FLAG;

    g_ctrl.frameToSend = buffer;

    unsigned int res = write(fd,buffer,5);
    if(res==5){
        unsigned char *buff = getReceptorResponse(fd);//get the response with the possibility of timeout
        if(buff==NULL){
            return 0;
        }
        unsigned int rez = ReceptorResponseInterpreter(buff);//check response type
        if(rez == UA_R){//if correct response type return 1
            free(buff);
            return 1;
        }else{
            free(buff);
            return 0;
        }
    }else{
        return 0;
    }
    return 0;
}

//NEEDS COMMENTING
unsigned char sendData(unsigned int fd,const unsigned char data[],unsigned int size){
    signal(SIGALRM,timeoutHandler);
    g_ctrl.retryCounter = 0;
    g_ctrl.fileDescriptor = fd;

    unsigned char**frames = NULL;
    unsigned int nFrames = allocateInformationFrames(frames,data,size);
    if(nFrames == 0){
        printf("[ERROR]@allocation:There has been an allocation error on the inforation frames\n");
        return 0;
    }
    prepareInformationFrames(frames,nFrames);
    moveDataToFrames(frames,data,size,nFrames);
    //the frames are now ready to be sent

    if(g_ctrl.allocError){
        printf("[ERROR]@allocation:There has been an allocation error while moving information to frames,exiting\n");
        return 0;
    }

    for(unsigned int i=0;i<nFrames;++i){
        unsigned int toSend = 0;
        if(i==nFrames-1){
            toSend = g_ctrl.lastFrameSize;
        }else{
            toSend = MAX_FRAME_SIZE;
        }
        g_ctrl.frameToSend = frames[i];
        unsigned int sent = write(fd,frames[i],toSend);
        if(sent==0){
            printf("Connection Error Unable To Senda Data\n");
            return 0;
        }
        unsigned char* buf = getReceptorResponse(fd);
        if(buf==NULL){
            return 0;
        }
        unsigned int rez = ReceptorResponseInterpreter(buf);

        if(g_ctrl.currPar==1){
            while(rez==REJ1){
                free(buf);
                buf = getReceptorResponse(fd);
                if(buf==NULL){
                    return 0;
                }
                rez = ReceptorResponseInterpreter(buf);
            }
            if(rez==RR1){
                return sent;
            }
        }else{
            while(rez==REJ0){
                free(buf);
                buf = getReceptorResponse(fd);
                if(buf==NULL){
                    return 0;
                }
                rez = ReceptorResponseInterpreter(buf);
            }
            if(rez==RR0){
                return sent;
            }
        }

        if(rez==UA_R){
            printf("Incorrect Response Type Received\n");
            return 0;
        }

        if(rez == ERR){
            printf("Corrupted Response Received\n");
            return 0;
        }
    }
    return 0;
}

void timeoutHandler(int sig){
    if(g_ctrl.retryCounter>=MAX_TIMEOUT){
        printf("Conection Timeout\n");
        g_ctrl.hasTimedOut = TRUE;
        return;
    }else{
        printf("Retrying Connection ... Attempt %d\n",g_ctrl.retryCounter+1);
        unsigned int res = write(g_ctrl.fileDescriptor,g_ctrl.frameToSend,5);
        if(res==0){
            printf("Conection Timeout\n");
            g_ctrl.hasTimedOut = TRUE;
            return;
        }
        g_ctrl.retryCounter++;
        alarm(TIMEOUT);
    }
}
