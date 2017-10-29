#define _USE_MATH_DEFINES

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

unsigned int hasReceived = FALSE;

unsigned int allocateInformationFrames(unsigned char ***buff, const unsigned char data[], unsigned int sizeOf)
{
    printf("[LOG]@memory\tStarting Allocation of information frames process for data of size:%d\n",sizeOf);
    //This bit of code determine how many frames will be necessary to send the data
    unsigned int size = (unsigned int)ceil((float)sizeOf / MAX_DATA_PER_FRAME);

    printf("[LOG]@memory\t Calculated %f required frames\n", ceil((float)sizeOf / MAX_DATA_PER_FRAME));

    //this allocates the buffer with the ammount of frames necessary to send the data
    *buff = (unsigned char **)malloc(sizeof(unsigned char *) * size);

    if (*buff == NULL)
    {
        printf("[ERROR]@memory\tAllocation of the information frame failed\n");
        deallocateInformationFrames(buff, 0); //since the was an error everything must be dealocated
        return -1;                            // this indicates there was an issue allocating the requires space
    }

    printf("[LOG]@memory\tAllocated information frame buffer of size %d\n", size);

    //this allocates the space required to send the data inside each frame
    for (unsigned int i = 0; i < size; ++i)
    {
        if (size > 1)
        {
            if (i == (size - 1))
            {
                double fraction = size - (sizeOf / MAX_DATA_PER_FRAME);                                //determines fraction of the max data per frame for the last frame
                g_ctrl.lastFrameSize = sizeof(unsigned char) * MAX_FRAME_SIZE * fraction;              //saves the size of the last frame for easy acess
                *buff[i] = (unsigned char *)malloc(sizeof(unsigned char) * MAX_FRAME_SIZE * fraction); //allocates the frame with only the needed size
                printf("[LOG]@memory\tAttempting to allocate an information frame of %f bytes\n", MAX_FRAME_SIZE * fraction);
            }
            else
            {
                *buff[i] = (unsigned char *)malloc(sizeof(unsigned char) * MAX_FRAME_SIZE);
                printf("[LOG]@memory\tAttempting to allocate an information frame of %d bytes\n", MAX_FRAME_SIZE);
            }
        }
        else
        {
            g_ctrl.lastFrameSize = (sizeOf + 6); 
            *buff[i] = (unsigned char *)malloc(sizeof(unsigned char) * (sizeOf + 6));
            printf("[LOG]@memory\tAttempting to allocate an information frame of %d bytes\n", (sizeOf + 6));
        }

        if (*buff[i] == NULL)
        {
            printf("[ERROR]@memory\tInformation Frame Failed to Allocate\n");
            deallocateInformationFrames(buff, i); //since the was an error everything must be dealocated
            return -1;                            // this indicates there was an issue allocating the requires space
        }

        printf("[SUCCES]@memory\tInformation Frame Successfully Allocated\n");
    }
    printf("[LOG]@memory\tAllocation of Information frames process ended\n");
    return size; //returns the ammount of frames allocated
}

void deallocateInformationFrames(unsigned char ***frames, unsigned int numberOfFrames)
{
    printf("[LOG]@memory\tStarting deallocation of information frames\n");
    //dealocates the memory space of each individual frame
    for (unsigned int i = 0; i < numberOfFrames; ++i)
    {
        free(*frames[i]);
    }
    //dealocates the frame buffer
    free(*frames);
    printf("[LOG]@memory\tFinished deallocating Information frames\n");
    return;
}

unsigned char **allocateCharBuffers(unsigned int numberOfBuffers, unsigned int dataPerBuffer)
{
    printf("[LOG]@memory\tStarting allocation of generic buffer\n");
    //allocates the buffer of buffers
    printf("[LOG]@memory\tAtempting to allocate buffer of size %d\n", numberOfBuffers);
    unsigned char **buffer = (unsigned char **)malloc(sizeof(unsigned char *) * numberOfBuffers);

    if (buffer == NULL)
    {
        printf("[ERROR]@memory\tFailed to allocated buffer\n");
        deallocatedCharBuffers(buffer, 0); //deallocates the buffer of buffers at is not needed
        return NULL;                       //indicates there has been an error with the allocation
    }
    printf("[SUCCESS]@memory\tBuffer holder allocated\n");

    //allocate each individual buffer
    for (unsigned int i = 0; i < numberOfBuffers; ++i)
    {
        buffer[i] = (unsigned char *)malloc(sizeof(unsigned char) * dataPerBuffer);
        printf("[LOG]@memory\tAttempting to allocated generic buffer of size %d\n", dataPerBuffer);

        if (buffer[i] == NULL)
        {
            printf("[ERROR]@memory\tFailed to allocated buffer\n");
            deallocatedCharBuffers(buffer, 0); //deallocates the buffer of buffers at is not needed
            return NULL;                       //indicates there has been an error with the allocation
        }
        printf("[SUCCESS]@memory\tBuffer allocated\n");
    }
    printf("[LOG]@memory\tgeneric buffer allocation finished\n");
    return buffer;
}

void deallocatedCharBuffers(unsigned char **buffers, unsigned int numberOfBuffers)
{
    printf("[LOG]@memory\tStarting generic buffer deallocation\n");
    //dealocates the memory space of each buffer
    for (unsigned int i = 0; i < numberOfBuffers; ++i)
    {
        free(buffers[i]);
    }
    printf("[LOG]@memory\tgeneric buffers freed\n");
    //delocates the buffer of buffers
    free(buffers);
    printf("[LOG]@memory\tFinsished generic buffer deallocation\n");
    return;
}

unsigned char **divideData(const unsigned char data[], unsigned int *sizeOf)
{
    printf("[LOG]@divData\tStarting data division process\n");
    unsigned int size = (unsigned int)ceil(*sizeOf / MAX_DATA_PER_FRAME);  //determine the size of the data
    unsigned char **buffer = allocateCharBuffers(size, MAX_DATA_PER_FRAME); //allocates the required space for the data holders

    if (buffer == NULL)
    {
        return NULL; //return an error code, meaning there was an error with the allocation
    }

    //moves each chunk of data to the buffers
    for (unsigned int i = 0; i < size; ++i)
    {
        if ((i * MAX_DATA_PER_FRAME + MAX_DATA_PER_FRAME) > *sizeOf)
        {
            unsigned int str = i * MAX_DATA_PER_FRAME;           //calculate the start of the range
            unsigned int end = *sizeOf - i * MAX_DATA_PER_FRAME; //calculate how much to copy at the end of the range
            memmove(buffer[i], data + str, end);
            printf("[LOG]@divData\tMoved data from range[%d,%d] to buffer\n", str, end);
            *sizeOf = *sizeOf - i * MAX_DATA_PER_FRAME;
        }
        else
        {
            memmove(buffer[i], data + i * MAX_DATA_PER_FRAME, MAX_DATA_PER_FRAME);
            printf("[LOG]@divData\tMoved data from range[%d,%d] to buffer\n", i * MAX_DATA_PER_FRAME, MAX_DATA_PER_FRAME);
        }
    }
    printf("[LOG]@divData\tFinished data division process\n");
    return buffer;
}

void moveInformationToFrame(unsigned char *frame, const unsigned char data[], unsigned int size)
{
    printf("[LOG]@movData\tStarting data move to frame\n");
    if (size > MAX_DATA_PER_FRAME * 2)
    {
        printf("[ERROR]@movData\tData to large for a single frame\n");
        return; //The data is too big for this frame;
    }

    memmove(frame + 4, data, size); //moves the data to the data section of the frame
    printf("[LOG]@movData\tData moved to frame\n");
    return;
}

void prepareInformationFrames(unsigned char **frames, unsigned int numberFrames)
{
    printf("[LOG]@frameSet\tFrame Set Up starded\n");
    for (unsigned int i = 0; i < numberFrames; ++i)
    {
        printf("[LOG]@frameSet\tsetting up frame %d\n", i);
        if (g_ctrl.currPar == 0)
        {
            g_ctrl.currPar = 1;
        }
        else
        {
            g_ctrl.currPar = 0;
        }

        unsigned int lastByte = 0;

        if (i == (numberFrames - 1))
        {
            lastByte = g_ctrl.lastFrameSize;
        }
        else
        {
            lastByte = MAX_FRAME_SIZE;
        }

        frames[i][0] = FLAG;                        //FLAG
        printf("[LOG]@frameSet\tFlag set\n");
        frames[i][1] = ADDRS;                       //ADDRS
        printf("[LOG]@frameSet\taddress set\n");
        frames[i][2] = g_ctrl.currPar;              //CTRL
        printf("[LOG]@frameSet\tcontroll set\n");
        frames[i][3] = frames[i][1] ^ frames[i][2]; //BCC1
        printf("[LOG]@frameSet\tbcc set\n");
        frames[i][lastByte - 1] = FLAG;             //FLAG
        printf("[LOG]@frameSet\tend flag set\n");
        printf("[LOG]@frameSet\tfinished setting up frame %d\n", i);
    }
    printf("[LOG]@frameSet\tFrame Set Up finished\n");
    return;
}

unsigned char moveDataToFrames(unsigned char **frames, const unsigned char data[], unsigned int size, unsigned int numberOfFrames)
{
    printf("[LOG]@dataMv\tStarting move of data to frame\n");
    unsigned int s_size = size;

    printf("[LOG]@dataMv\tAttempting to divide data\n");
    unsigned char **info = divideData(data, &s_size); //obtain the data divided into chunks

    if (info == NULL)
    {
        printf("[ERROR]@dataMv\tFailed to split data into chunks\n");
        return 0;
    }
    else
    {
        printf("[SUCCESS]@dataMv\tData has been split\n");
    }

    for (unsigned int i = 0; i < numberOfFrames; ++i)
    {
        unsigned int sizeOf = 0; //size of the current frame
        if (i == numberOfFrames - 1)
        {
            sizeOf = g_ctrl.lastFrameSize;
        }
        else
        {
            sizeOf = MAX_FRAME_SIZE;
        }

        printf("[LOG]@dataMv\tAttempting to bytte stuff data\n");
        unsigned char *stuffed = byteStuffingOnData(info[i], &sizeOf);
        s_size = sizeOf;
        if (stuffed == NULL)
        {
            printf("[ERROR]@dataMv\tByte stuffing failed\n");
            g_ctrl.allocError = 1;
            return 0;
        }
        else
        {
            printf("[SUCCESS]@dataMv\tByte stuffing complete\n");
        }
        g_ctrl.allocError = 0;
        moveInformationToFrame(frames[i], stuffed, s_size);     //moves the chunk of data into the frame
        frames[i][sizeOf - 2] = calculateBCC2(info[i], s_size); //sets the BCC for the data chunk that was moved
    }
    printf("[SUCCESS]@dataMv\tInformation and BCC2 have been set for the frames\n");
    return 1;
}

unsigned char calculateBCC2(const unsigned char data[], unsigned int sizeOfData)
{
    printf("[LOG]@bcc2\tStarting BCC2 calculation\n");
    unsigned char ret = 0;
    unsigned int size = sizeOfData / sizeof(unsigned char); //determine the size of the data

    for (unsigned int i = 0; i < size; ++i)
    {
        ret = ret ^ data[i]; //calculate the XOR of the whole data
    }
    printf("[LOG]@bcc2\tFinished BCC2 calculation\n");
    return ret;
}

//Need to document and properly adapt the code
unsigned int openConnection(char *serialPort, unsigned int flags)
{

    printf("[LOG]@openc\tOpening Connection to serial port:%s\n", serialPort);

    if (((strcmp("/dev/ttyS0", serialPort) != 0) && (strcmp("/dev/ttyS1", serialPort) != 0)))
    {
        printf("[ERROR]@openc\tInvalid serial port supplied\n");
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }

    printf("[LOG]@openc\tAttempting to open file descriptor\n");
    int fd = -1;
    unsigned int fdz = -1;

    if (flags == 0)
    {
        fd = open(serialPort, O_RDWR | O_NOCTTY);
    }
    else
    {
        fd = open(serialPort, flags);
    }

    if (fd < 0)
    {
        printf("[ERROR]@openc\tFile descriptor failed to open\n");
        perror(serialPort);
        exit(-1);
    }
    else
    {
        printf("[SUCCESS]@openc\tFile descriptor obatained\n");
        fdz = fd;
    }

    printf("[LOG]@openc\tAttempting to  obtain file descriptor current attributes\n");
    if (tcgetattr(fdz, &oldtio) == -1)
    { /* save current port settings */
        printf("[ERROR]@openc\tFailed to obtain file descriptor current attributes\n");
        perror("tcgetattr");
        exit(-1);
    }
    else
    {
        printf("[SUCCESS]@openc\tObtained file descriptor attributes\n");
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 5;  /* blocking read until 5 chars received */

    tcflush(fdz, TCIOFLUSH);

    printf("[LOG]@openc\tAttempting to set file descriptor current attributes\n");
    if (tcsetattr(fdz, TCSANOW, &newtio) == -1)
    {
        printf("[ERROR]@openc\tFailed to set file descriptor current attributes\n");
        perror("tcsetattr");
        return -1;
    }
    else
    {
        printf("[SUCCESS]@openc\tSet file descriptor current attributes\n");
    }

    printf("New termios structure set\n");
    printf("[LOG]@openc\tFinished estableshing serial porto conenction to:%s\n", serialPort);
    return fdz;
}

void closeConnection(unsigned int fd)
{
    printf("[LOG]@closec\tAttempting to close serial port file descriptor\n");
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        printf("[ERROR]@closec\tFailed to close serial port file descriptor\n");
        perror("tcsetattr");
        exit(-1);
    }
    else
    {
        printf("[SUCCESS]@closec\tClosed serial port file descriptor\n");
    }

    close(fd);
}

//NEED TO DOCUMENT - MISSING ERROR CHECKING
unsigned char *byteStuffingOnData(const unsigned char data[], unsigned int *sizeOfData)
{
    printf("[LOG]@stuff\tStarting byte stuffing proccess\n");
    unsigned int originalSize = *sizeOfData;
    unsigned int postSize = originalSize;

    printf("[LOG]@stuff\tAttempting to allocated buffers\n");
    unsigned char **buffer = allocateCharBuffers(1, originalSize * 2); // we really should not use this but as a barebones implementation it can pass

    if (buffer == NULL)
    {
        printf("[ERROR]@stuff\tFailed to allocated buffer\n");
        return NULL;
    }
    else
    {
        printf("[SUCCESS]@stuff\tBuffers allocated\n");
    }

    unsigned int currIndice = 0;
    for (unsigned int i = 0; i < originalSize; ++i)
    {
        if (data[i] == 0x7D)
        {
            printf("[LOG]@stuff\t0x7D detected to stuff\n");
            buffer[0][currIndice] = 0x7D;
            buffer[0][++currIndice] = 0x5D;
            postSize++;
            printf("[LOG]@stuff\tStuffed\n");
        }
        else if (data[i] == 0x7E)
        {
            printf("[LOG]@stuff\t0x7E detected to stuff\n");
            buffer[0][currIndice] = 0x7D;
            buffer[0][++currIndice] = 0x5E;
            postSize++;
            printf("[LOG]@stuff\tStuffed\n");
        }
        else
        {
            printf("[LOG]@stuff\tdata section does not need stuff\n");
            buffer[0][currIndice] = data[i];
        }
        printf("[LOG]@stuff\tcurrentIndice %d\n",currIndice);
        currIndice++;
    }

    printf("[LOG]@stuff\tAttempting to resize buffer\n");
    void *ret = realloc(buffer[0], postSize);
    if (ret == NULL)
    {
        printf("[ERROR]@stuff\tFailed to resize buffer\n");
        return NULL;
    }
    else
    {
        printf("[SUCCESS]@stuff\tResized the buffer\n");
    }
    *sizeOfData = postSize;

    printf("[LOG]@stuff\tFinished stuffing\n");
    return buffer[0];
}

//NEEDS TO BE DOCUMENTED
char ReceptorResponseInterpreter(const unsigned char *receptorResponse)
{
    printf("[LOG]@interpret\tStarting response interpreter\n");
    unsigned int currntState = FLAG_STR;
    unsigned int responseType;

    for (unsigned int i = 0; i < 5; ++i)
    {
        switch (currntState)
        {

        case (FLAG_STR):
        {
            if (receptorResponse[i] == FLAG)
            {
                currntState = ADDR;
            }
            else
            {
                printf("[LOG]@interpret\tInvalid Frame!\n");
                return ERR;
            }
            break;
        }

        case (ADDR):
        {
            if (receptorResponse[i] == ADDR2)
            {
                currntState = CTRLL;
            }
            else
            {
                printf("[LOG]@interpret\tInvalid Frame!\n");
                return ERR;
            }
            break;
        }

        case (CTRLL):
        {
            if (g_ctrl.currPar == 0)
            {
                if (receptorResponse[i] == UA)
                {
                    responseType = UA_R;
                    currntState = BCC;
                    break;
                }
                else if (receptorResponse[i] == RR0)
                {
                    responseType = ACPT;
                    currntState = BCC;
                    break;
                }
                else if (receptorResponse[i] == REJ0)
                {
                    responseType = REJ;
                    currntState = BCC;
                    break;
                }
                else
                {
                    printf("[LOG]@interpret\tInvalid Frame!\n");
                    return ERR;
                }
            }
            else if (g_ctrl.currPar == 1)
            {
                if (receptorResponse[i] == UA)
                {
                    responseType = UA_R;
                    currntState = BCC;
                    break;
                }
                else if (receptorResponse[i] == RR1)
                {
                    responseType = ACPT;
                    currntState = BCC;
                    break;
                }
                else if (receptorResponse[i] == REJ1)
                {
                    responseType = REJ;
                    currntState = BCC;
                    break;
                }
                else
                {
                    printf("[LOG]@interpret\tInvalid Frame!\n");
                    return ERR;
                }
            }
            break;
        }

        case (BCC):
        {
            if (receptorResponse[i] == (receptorResponse[i - 1] ^ receptorResponse[i - 2]))
            {
                currntState = FLAG_END;
                break;
            }
            else
            {
                printf("[LOG]@interpret\tInvalid Frame!\n");
                return ERR;
            }
        }

        case (FLAG_END):
        {
            if (receptorResponse[i] == FLAG)
            {
                currntState = DONE_PROC;
                break;
            }
            else
            {
                printf("[LOG]@interpret\tInvalid Frame!\n");
                return ERR;
            }
        }

        default:
            break;
        }
    }

    if (currntState == DONE_PROC)
    {
        printf("[LOG]@interpret\tFinished response interpreting\n");
        return responseType;
    }
    printf("[LOG]@interpret\tInvalid Frame!\n");
    return ERR; //error code
}

//NEEDS TO BE COMMENTED
unsigned char *getReceptorResponse(unsigned int fd)
{
    hasReceived = FALSE;
    g_ctrl.retryCounter = 0;
    signal(SIGALRM, timeoutHandler);
    printf("[LOG]@rdR\tStarting Receptor Response Read Cycle\n");
    unsigned int res = 0;

    printf("[LOG]@rdR\tAttempting to allocate buffer\n");
    unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char) * 5);
    if (buffer == NULL)
    {
        printf("[ERROR]@rdR\tFailed to allocate buffer\n");
        return NULL;
    }
    else
    {
        printf("[SUCCESS]@rdR\tBuffer was allocated\n");
    }

    printf("[LOG]@rdR\tTimeout alarm engaged\n");
    alarm(TIMEOUT); //sets an alarm with the timeout value , to manage connection timeouts
    while (res != 5)
    {
        printf("[LOG]@rdR\tAttempting to read response\n");
        res = read(fd, buffer, 5);
        if (res == 5)
        { // if it reads it disables the alarms and marks the connection as successful and returns 1
            hasReceived = TRUE;
            printf("[LOG]@rdR\tResponse Detected\n");
            alarm(0);
            g_ctrl.hasTimedOut = FALSE;
            g_ctrl.retryCounter = 0;
            printf("[LOG]@rdR\tDone attempting to read response\n");
            return buffer;
        }
        else
        {
            printf("[LOG]@rdR\tDidnt read 5 bytes, read:\n", res);
            write(g_ctrl.fileDescriptor, g_ctrl.frameToSend, g_ctrl.sendSize);
            sleep(TIMEOUT);
        }
        if (g_ctrl.hasTimedOut)
        { // if the connection is marked as timed out 0 is returned
            printf("[ERROR]@rdR\tFailed to read receptor response\n");
            return NULL;
        }
    }
    printf("[ERROR]@rdR\tFailed to read receptor response\n");
    return NULL;
}

//NEEDS TO BE COMMENTED
unsigned char sendSetCommand(unsigned int fd)
{
    printf("[LOG]@stSend\tStarting send command transmission proccess\n");
    signal(SIGALRM, timeoutHandler);
    g_ctrl.currPar = 0;
    g_ctrl.retryCounter = 0;
    g_ctrl.fileDescriptor = fd;

    //configures a buffer with the information for the set command
    unsigned char buffer[5];
    buffer[0] = FLAG;
    buffer[1] = ADDRS;
    buffer[2] = SET;
    buffer[3] = buffer[1] ^ buffer[2];
    buffer[4] = FLAG;

    g_ctrl.frameToSend = buffer;
    g_ctrl.sendSize = 5;

    printf("[LOG]@stSend\tAttempting to send frame\n");
    unsigned int res = write(fd, buffer, 5); //writes to pipe the set command in the buffer

    unsigned char *buff;

jump:
    if (res == 5)
    {
        printf("[LOG]@stSend\tFrame sent , attempting to read response\n");
        g_ctrl.retryCounter = 0;        //if it wrote the whole buffer it will now atemp to read the confirmation for the receptor
        buff = getReceptorResponse(fd); //get the response with the possibility of timeout
        if (buff == NULL)
        {
            printf("[ERROR]@stSend\tResponse timeout\n");
            return 0;
        }
        unsigned int rez = ReceptorResponseInterpreter(buff); //check response type
        if (rez == UA_R)
        { //if correct response type return 1
            printf("[LOG]@stSend\tValid response received\n");
            free(buff);
            return 1;
        }
        else
        { //invalid reponse
            printf("[ERROR]@stSend\tInvalid Response received\n");
            free(buff);
            return 0;
        }
    }
    else
    {
        while (res != 5 && g_ctrl.retryCounter <= MAX_TIMEOUT)
        {
            printf("[ERROR]@stSend\tFrame was not sent , attempting retransmission...\n");
            sleep(TIMEOUT);
            res = write(fd, buffer, 5);
            g_ctrl.retryCounter++;
        }
        if (res == 5)
        {
            printf("[SUCCESS]@stSend\tRetransmission successfull\n");
            goto jump;
        }
        return 0;
    }
    return 0;
}

unsigned char sendDisconnectCommand(unsigned int fd)
{
    printf("[LOG]@dcSend\tStarting diconnect command sending proccess\n");
    signal(SIGALRM, timeoutHandler);
    g_ctrl.retryCounter = 0;
    g_ctrl.fileDescriptor = fd;

    unsigned char buffer[5];
    buffer[0] = FLAG;
    buffer[1] = ADDRS;
    buffer[2] = DISC;
    buffer[3] = buffer[1] ^ buffer[2];
    buffer[4] = FLAG;

    g_ctrl.frameToSend = buffer;
    g_ctrl.sendSize = 5;

    printf("[LOG]@dcSend\tAttemprting to send frame\n");
    unsigned int res = write(fd, buffer, 5);
jump:
    if (res == 5)
    {
        printf("[LOG]@dcSend\tFrame sent ,a ttempting to read response\n");
        g_ctrl.retryCounter = 0;
        unsigned char *buff = getReceptorResponse(fd); //get the response with the possibility of timeout
        if (buff == NULL)
        {
            printf("[ERROR]@dcSend\tFailed to read response\n");
            return 0;
        }
        unsigned int rez = ReceptorResponseInterpreter(buff); //check response type
        if (rez == UA_R)
        { //if correct response type return 1
            printf("[LOG]@dcSend\tValid response received\n");
            free(buff);
            return 1;
        }
        else //incorrect response
        {
            printf("[ERROR]@dcSend\tInvalid response received\n");
            free(buff);
            return 0;
        }
    }
    else
    {
        while (res != 5 && g_ctrl.retryCounter <= MAX_TIMEOUT)
        {
            printf("[ERROR]@stSend\tFrame was not sent , attempting retransmission...\n");
            sleep(TIMEOUT);
            res = write(fd, buffer, 5);
            g_ctrl.retryCounter++;
        }
        if (res == 5)
        {
            printf("[SUCCESS]@stSend\tRetransmission successfull\n");
            goto jump;
        }
        return 0;
    }
    return 0;
}

//NEEDS COMMENTING
unsigned char sendData(unsigned int fd, const unsigned char data[], unsigned int size)
{
    printf("[LOG]@dataSend\tStarting data sending proccess\n");
    signal(SIGALRM, timeoutHandler);
    g_ctrl.retryCounter = 0;
    g_ctrl.fileDescriptor = fd;

    printf("[LOG]@dataSend\tAttempting to allocated frame buffers\n");
    unsigned char **frames = NULL;
    unsigned int nFrames = allocateInformationFrames(&frames, data, size);
    if (nFrames == 0 || frames == NULL)
    {
        printf("[ERROR]@allocation:There has been an allocation error on the inforation frames\n");
        return 0;
    }
    printf("[LOG]@dataSend\tAttempting to preprare information frames\n");
    prepareInformationFrames(frames, nFrames); //this function does not error in normal situations

    printf("[LOG]@dataSend\tAttempting to move data to frames\n");
    unsigned char ret = moveDataToFrames(frames, data, size, nFrames);
    if (ret == 0)
    {
        printf("[ERROR]@dataSend\tMoving data to frames failed\n");
        return 0;
    }
    else
    {
        printf("[LOG]@dataSend\tData moved to frames\n");
    }

    //the frames are now ready to be sent

    if (g_ctrl.allocError)
    {
        printf("[ERROR]@dataSend\tThere has been an allocation error while moving information to frames,exiting\n");
        return 0;
    }

    for (unsigned int i = 0; i < nFrames; ++i)
    {
        unsigned int toSend = 0;
        if (i == nFrames - 1)
        {
            toSend = g_ctrl.lastFrameSize;
        }
        else
        {
            toSend = MAX_FRAME_SIZE;
        }
        g_ctrl.frameToSend = frames[i];
        g_ctrl.sendSize = toSend;

    resend:
        printf("[LOG]@dataSend\tAttempting to send data\n");
        unsigned int sent = write(fd, frames[i], toSend);

        if (g_ctrl.retryCounter > MAX_TIMEOUT + 1)
        {
            printf("[ERROR]@dataSend\tMaximum permited retransmission executed, connection timeout\n");
            return 0;
        }

        while (sent != toSend && g_ctrl.retryCounter <= MAX_TIMEOUT)
        {
            sleep(TIMEOUT);
            printf("[ERROR]@dataSend\tFailed to send data, attempting retansmission\n");
            g_ctrl.retryCounter++;
            goto resend;
        }

        g_ctrl.retryCounter = 0;

        printf("[LOG]@dataSend\tAttempting to obtain receptor response\n");
        unsigned char *buf = getReceptorResponse(fd);
        if (buf == NULL)
        {
            printf("[ERROR]@dataSend\tFailed to get response from receiver , connection timeout\n");
            return 0;
        }
        else
        {
            printf("[LOG]@dataSend\tObtained receptor response\n");
        }

        printf("[LOG]@dataSend\tAttempting interpret receptor response\n");
        unsigned int rez = ReceptorResponseInterpreter(buf);

        if (g_ctrl.currPar == 1)
        {
            while (rez == REJ1)
            {
                free(buf);
                printf("[ERROR]@dataSend\tReceived response of corrupted frame\n");
                goto resend;
            }
            if (rez == RR1)
            {
                printf("[LOG]@dataSend\tReceived response of valid frame\n");
                free(buf);
                return sent;
            }
        }
        else
        {
            while (rez == REJ0)
            {
                free(buf);
                printf("[ERROR]@dataSend\tReceived response of corrupted frame\n");
                goto resend;
            }
            if (rez == RR0)
            {
                printf("[LOG]@dataSend\tReceived response of valid frame\n");
                free(buf);
                return sent;
            }
        }

        if (rez == UA_R)
        {
            printf("[ERROR]@dataSend\tIncorrect response type received\n");
            return 0;
        }

        if (rez == ERR)
        {
            printf("[ERROR]@dataSend\tCorrupted response received\n");
            return 0;
        }
    }
    return 0;
}

void timeoutHandler(int sig)
{
    signal(SIGALRM,timeoutHandler);
    if (hasReceived == FALSE)
    {
        printf("[LOG]@timeoutHandle\tTimeout signal occurred\n");
        if (g_ctrl.retryCounter >= MAX_TIMEOUT)
        {
            printf("[ERROR]@timeoutHandle\tConnection timeout\n");
            g_ctrl.hasTimedOut = TRUE;
            return;
        }
        else
        {
            unsigned int resend = 0;
            printf("[LOG]@timeoutHandle\tRetrying Connection ... Attempt %d\n", g_ctrl.retryCounter + 1);
            unsigned int res = write(g_ctrl.fileDescriptor, g_ctrl.frameToSend, g_ctrl.sendSize);
            while (res != 5)
            {
                sleep(TIMEOUT);
                printf("[ERROR]@timeoutHandle\tFailed to resend, retrying retransmission\n");
                res = write(g_ctrl.fileDescriptor, g_ctrl.frameToSend, 5);
                resend++;
                if (resend > MAX_TIMEOUT + 1)
                {
                    printf("[ERROR]@timeoutHandle\tAll retransmission failed , aborting\n");
                    g_ctrl.hasTimedOut = TRUE;
                    return;
                }
            }
            g_ctrl.retryCounter++;
            alarm(TIMEOUT);
        }
    }
}

unsigned char *destuffData(unsigned char *data, unsigned int *sizeOf)
{
    printf("[LOG]@destuffing\tStarting Data Destuffing Proccess\n");
    unsigned int initialSize = *sizeOf;
    unsigned int currSize = *sizeOf;

    unsigned int currInd = 0;

    unsigned char *retData = (unsigned char *)malloc(sizeof(unsigned char) * initialSize);
    if (retData == NULL)
    {
        printf("[ERROR]@destuffing\tFailed to allocated buffer for destuffed data\n");
        return NULL;
    }

    for (unsigned int i = 0; i < initialSize; ++i)
    {
        if (data[i] == 0x7D)
        {
            if (data[i + 1] == 0x5d)
            {
                retData[currInd] = 0x7D;
                currSize--;
                i++;
            }
            else if (data[i + 1] == 0x5E)
            {
                retData[currInd] = 0x7E;
                currSize--;
                i++;
            }
            else
            {
                retData[currInd] = data[i];
            }
            currInd++;
        }
        else
        {
            retData[currInd] = data[i];
            currInd++;
        }
    }

    void *chk = realloc(retData, currSize);
    if (chk == NULL)
    {
        printf("[ERROR]@destuffing\tFailed to resize destuffed data buffer\n");
        return NULL;
    }

    *sizeOf = currSize;

    printf("[LOG]@destuffing\tFinished Data Destuffing Proccess\n");
    return retData;
}

unsigned char sendRRCommand(unsigned int fd)
{
    printf("[LOG]@cmdRec\tStarting RR commander sending proccess\n");
    unsigned char buff[5];

    printf("[LOG]@cmdRec\tpreparing command buffer\n");
    buff[0] = FLAG;
    buff[1] = ADDR2;
    if (g_ctrl.currPar == 0)
    {
        buff[2] = RR0;
        g_ctrl.currPar = 1;
    }
    else
    {
        buff[2] = RR1;
        g_ctrl.currPar = 0;
    }
    buff[3] = buff[1] ^ buff[2];
    buff[4] = FLAG;
    printf("[LOG]@cmdRec\tCommand buffer ready\n");

    unsigned int retryCounter = 0;
    unsigned int sent = 0;
    while (sent == 0)
    {
        retryCounter++;

        printf("[LOG]@cmdRec\tSending Command Buffer\n");
        sent = write(fd, buff, sizeof(buff));

        if (sent != sizeof(buff))
        {
            sleep(TIMEOUT);
        }

        if (retryCounter > (MAX_TIMEOUT + 1) && sent != sizeof(buff))
        {
            printf("[ERROR]@cmdRec\tFailed to send command to specified file descriptor more than %d times\n", retryCounter);
            return 0;
        }
    }

    if (sent == 5)
    {
        printf("[SUCCESS]@cmdRec\tSent RR Command Successfully\n");
        return 1;
    }

    return 0;
}

unsigned char sendREJCommand(unsigned int fd)
{
    printf("[LOG]@cmdRec\tStarting REJ commander sending proccess\n");
    unsigned char buff[5];

    printf("[LOG]@cmdRec\tpreparing command buffer\n");
    buff[0] = FLAG;
    buff[1] = ADDR2;
    if (g_ctrl.currPar == 0)
    {
        buff[2] = REJ0;
    }
    else
    {
        buff[2] = REJ1;
    }
    buff[3] = buff[1] ^ buff[2];
    buff[4] = FLAG;
    printf("[LOG]@cmdRec\tCommand buffer ready\n");

    unsigned int retryCounter = 0;
    unsigned int sent = 0;
    while (sent == 0)
    {
        retryCounter++;

        printf("[LOG]@cmdRec\tSending Command Buffer\n");
        sent = write(fd, buff, sizeof(buff));

        if (sent != sizeof(buff))
        {
            sleep(TIMEOUT);
        }

        if (retryCounter > (MAX_TIMEOUT + 1) && sent != sizeof(buff))
        {
            printf("[ERROR]@cmdRec\tFailed to send command to specified file descriptor more than %d times\n", retryCounter);
            return 0;
        }
    }

    if (sent == 5)
    {
        printf("[SUCCESS]@cmdRec\tSent REJ Command Successfully\n");
        return 1;
    }

    return 0;
}

unsigned char sendUACommand(unsigned int fd)
{
    printf("[LOG]@cmdRec\tStarting UA commander sending proccess\n");
    unsigned char buff[5];

    printf("[LOG]@cmdRec\tpreparing command buffer\n");
    buff[0] = FLAG;
    buff[1] = ADDR2;
    buff[2] = UA;
    buff[3] = buff[1] ^ buff[2];
    buff[4] = FLAG;
    printf("[LOG]@cmdRec\tCommand buffer ready\n");

    unsigned int retryCounter = 0;
    unsigned int sent = 0;
    while (sent == 0)
    {
        retryCounter++;

        printf("[LOG]@cmdRec\tSending Command Buffer\n");
        sent = write(fd, buff, sizeof(buff));

        if (sent != sizeof(buff))
        {
            sleep(TIMEOUT);
        }

        if (retryCounter > (MAX_TIMEOUT + 1) && sent != sizeof(buff))
        {
            printf("[ERROR]@cmdRec\tFailed to send command to specified file descriptor more than %d times\n", retryCounter);
            return 0;
        }
    }

    if (sent == 5)
    {
        printf("[SUCCESS]@cmdRec\tSent UA Command Successfully\n");
        return 1;
    }

    return 0;
}

unsigned char *extractDataFromFrame(unsigned char *data, unsigned int *sizeOf)
{
    printf("[LOG]@extract\tStarting Data Extraction proccess\n");

    unsigned int sz = (*sizeOf - 6); //size of the data section in the frame

    printf("[LOG]@extract\tAtempting to allocate buffer for data of size:%d\n", sz);
    unsigned char *buff = (unsigned char *)malloc(sizeof(unsigned char) * sz);

    if (buff == NULL)
    {
        printf("[ERROR]@extract\tFailed To allocate buffer for data\n");
        return NULL;
    }
    else
    {
        printf("[SUCCESS]@extract\tSuccessfully allocated buffer for data\n");
    }

    memmove(buff, data + 4, sz);
    *sizeOf = sz;
    printf("[SUCCESS]@extract\tData Extraction Complete\n");
    return buff;
}

void readtimeoutHandler(int sig){
    signal(SIGALRM,readtimeoutHandler);
    if(hasReceived==FALSE){
        g_ctrl.retryCounter++;
        if(g_ctrl.retryCounter>MAX_TIMEOUT+1){
            printf("[ERROR]@rdTimehourHandler\tTimeout the read\n");
            g_ctrl.hasTimedOut=TRUE;
            return;
        }else{
            printf("[LOG]@rdTimehourHandler\tTimeout period reached , retrying attempt %d\n",g_ctrl.retryCounter);
            alarm(TIMEOUT);
        }
    }
    return;
}

unsigned char *readSentData(unsigned int fd, unsigned int *sizeOf)
{
    signal(SIGALRM,readtimeoutHandler);
    hasReceived=FALSE;
    printf("[LOG]@rcRd\tStarting to read sent data\n");

    //unsigned int readFlag1 = 0;
    //unsigned int readFlag2 = 0;
    //unsigned int stop = 0;

    printf("[LOG]@rcRd\tAttempting to allocate buffer with max frame size\n");
    unsigned char *buff = (unsigned char *)malloc(sizeof(unsigned char) * MAX_FRAME_SIZE);

    if (buff == NULL)
    {
        printf("[ERROR]@rcRd\tFailed to allocated buffer\n");
        return NULL;
    }
    else
    {
        printf("[SUCCESS]@rcRd\tSuccessfully allocated buffer\n");
    }

    unsigned char rd;
    unsigned int sz = 0;

    printf("[LOG]@rcRd\tReading Data\n");

    unsigned int retry = 0;
    unsigned int currSts = FLAG_STR;
    while (currSts != DONE_PROC && retry <= MAX_TIMEOUT + 1)
    {
        hasReceived=FALSE;
        alarm(TIMEOUT);
        if(g_ctrl.hasTimedOut==TRUE){
            printf("[ERROR]@rcRd\tRead timeout\n");
            return NULL;
        }
        unsigned int res = read(fd, &rd, 1);
        if (res == 1)
        {
            hasReceived=TRUE;
            g_ctrl.retryCounter=0;
            alarm(0);
            retry = 0;
            printf("[LOG]@rcRd\tRead a byte of data\n");
            buff[sz] = rd;
            switch (currSts)
            {
            case (FLAG_STR):
            {
                if (rd == FLAG)
                {
                    printf("[LOG]@rcRd\tStart Flag read\n");
                    currSts = ADDR;
                    sz++;
                }
                else
                {
                    printf("[ERROR]@rcRd\tStart Flag not read\n");
                    currSts = FLAG_STR;
                    sz = 0;
                }
                break;
            }
            case (ADDR):
            {
                if (rd == ADDRS)
                {
                    printf("[LOG]@rcRd\tAddress byte read\n");
                    currSts = CTRLL;
                    sz++;
                }
                else
                {
                    printf("[ERROR]@rcRd\tAddress byte not read\n");
                    currSts = FLAG_STR;
                    sz = 0;
                }
                break;
            }
            case (CTRLL):
            {
                if (rd == SET || rd == DISC || rd == CTR_PAR0 || rd == CTR_PAR1)
                {
                    printf("[LOG]@rcRd\tControll byte read\n");
                    currSts = BCC;
                    sz++;
                }
                else
                {
                    printf("[LOG]@rcRd\tControll byte noy read\n");
                    currSts = FLAG_STR;
                    sz = 0;
                }
                break;
            }
            case (BCC):
            {
                if (rd == (buff[sz - 1] ^ buff[sz - 2]))
                {
                    if (buff[sz - 1] == CTR_PAR0 || buff[sz - 1] == CTR_PAR1)
                    {
                        printf("[LOG]@rcRd\tBCC byte read,reading information\n");
                        currSts = INFO_STT;
                    }
                    else
                    {
                        printf("[LOG]@rcRd\tBCC byte read,reading end byte\n");
                        currSts = FLAG_END;
                    }
                    sz++;
                }
                else
                {
                    printf("[ERROR]@rcRd\tBCC byte not validated\n");
                    currSts = FLAG_STR;
                    sz = 0;
                }
                break;
            }
            case (INFO_STT):
            {
                if (rd == FLAG)
                {
                    printf("[LOG]@rcRd\tRead end flag\n");
                    currSts = DONE_PROC;
                    sz++;
                }
                else
                {
                    printf("[LOG]@rcRd\tRead information byte\n");
                    sz++;
                }
                break;
            }
            case (FLAG_END):
            {
                if (rd == FLAG)
                {
                    printf("[LOG]@rcRd\tRead End Flag\n");
                    currSts = DONE_PROC;
                    sz=5;
                }
                else
                {
                    printf("[ERROR]@rcRd\tFailed to read end flag\n");
                    currSts = FLAG_STR;
                    sz = 0;
                }
                break;
            }
            default:
                break;
            }
            if (currSts == DONE_PROC)
            {
                break;
            }
        }
        else
        {
            retry++;
            sleep(TIMEOUT);
            //timeout system
        }
    }

    /*while (!stop)
    {
        unsigned int res = read(fd, &rd, 1);
        if (res == 1)
        {

            if (sz > MAX_FRAME_SIZE && stop == 0)
            {
                printf("[ERROR]@rcRd\tToo much data read without escape Flag\n");
                return NULL;
            }

            printf("[LOG]@rcRd\tRead One Byte\n");
            if (readFlag1 == 0 && rd == FLAG)
            {
                printf("[LOG]@rcRd\tRead Start Frame Flag Byte\n");
                readFlag1 = 1;
                buff[sz] = rd;
                sz++;
                continue;
            }
            if (readFlag1 == 1 && readFlag2 == 0 && rd!=FLAG)
            {
                printf("[LOG]@rcRd\tRead Intermediate Byte Byte\n");
                buff[sz] = rd;
                sz++;
                continue;
            }
            if (readFlag1 == 1 && readFlag2 == 0 && rd == FLAG)
            {
                printf("[LOG]@rcRd\tRead End Frame Flag Byte\n");
                readFlag2 = 1;
                buff[sz] = rd;
                sz++;
                stop = 1;
            }
        }
}*/
    printf("[LOG]@rcRd\tFinished reading Data\n");

    printf("[LOG]@rcRd\tAttempting to resize frame buffer\n");
    void *ck = realloc(buff, sz);

    if (ck == NULL)
    {
        printf("[ERROR]@rcRd\tFailed to resize frame buffer\n");
        return NULL;
    }
    else
    {
        printf("[SUCCESS]@rcRd\tSuccessfully resized buffer\n");
    }

    *sizeOf = sz;
    return buff;
}

unsigned char validateFrame(unsigned char *data, unsigned int sizeOf)
{
    printf("[LOG]@valid\tStarting Frame Validation\n");

    if (sizeOf < 5 || sizeOf == 6)
    { // impossible for a valid frame to be smaller than 5 or size 6
        printf("[ERROR]@valid\tFrame Is Too Small to be a valid Frame\n");
        return 0;
    }

    //Means its a command frame
    if (sizeOf == 5)
    {
        printf("[LOG]@valid\tStarting Frame Validation for command frame\n");

        unsigned int currStt = FLAG_STR;
        unsigned int set = 0;
        unsigned int dis = 0;

        for (unsigned int i = 0; i < 5; ++i)
        {
            switch (currStt)
            {
            case (FLAG_STR):
            {
                if (data[i] == FLAG)
                {
                    printf("[SUCCESS]@valid\tStart Flag field is validated\n");
                    currStt = ADDR;
                    break;
                }
                else
                {
                    printf("[ERROR]@valid\tStart Byte does not contain Flag\n");
                    return 0;
                }
            }
            case (ADDR):
            {
                if (data[i] == ADDRS)
                {
                    printf("[SUCCESS]@valid\tAddress field is validated\n");
                    currStt = CTRLL;
                    break;
                }
                else
                {
                    printf("[ERROR]@valid\tAddress field is incorrect\n");
                    return 0;
                }
            }
            case (CTRLL):
            {
                if (data[i] == SET)
                {
                    printf("[SUCCESS]@valid\tControll field is validated as set command\n");
                    currStt = BCC;
                    set = 1;
                    break;
                }
                else if (data[i] == DISC)
                {
                    printf("[SUCCESS]@valid\tControll field is validated as disc command\n");
                    currStt = BCC;
                    dis = 1;
                    break;
                }
                else
                {
                    printf("[ERROR]@valid\tControll field is incorrect\n");
                    return 0;
                }
            }
            case (BCC):
            {
                if (data[i] == (data[i - 1] ^ data[i - 2]))
                {
                    printf("[SUCCESS]@valid\tBCC field is validated\n");
                    currStt = FLAG_END;
                    break;
                }
                else
                {
                    printf("[ERROR]@valid\tBCC field is incorrect\n");
                    return 0;
                }
            }
            case (FLAG_END):
            {
                if (data[i] == FLAG)
                {
                    printf("[SUCCESS]@valid\tEnd Flag field is validated\n");
                    currStt = DONE_PROC;
                    break;
                }
                else
                {
                    printf("[ERROR]@valid\tEnd Flag is incorrect\n");
                    return 0;
                }
            }
            }
        }

        printf("[LOG]@valid\tFinished Frame Validation for command frame\n");

        if (currStt == DONE_PROC)
        {
            if (set == 1)
            {
                printf("[LOG]@valid\tValidated a SET Command\n");
                setHandler();
                return 1;
            }
            else if (dis == 1)
            {
                printf("[LOG]@valid\tValidated a DISC Command\n");
                discHandler();
                return 1;
            }

            printf("[ERROR]@valid\tUnrecognized Command Received\n");
            return 0;
        }
    }

    //means its an information frame
    if (sizeOf >= 7)
    {
        printf("[LOG]@valid\tStarting Frame Validation for information frame\n");

        unsigned int currStt = FLAG_STR;

        for (unsigned int i = 0; i < sizeOf; ++i)
        {
            switch (currStt)
            {
            case (FLAG_STR):
            {
                if (data[i] == FLAG)
                {
                    printf("[SUCCESS]@valid\tStart Flag field is validated\n");
                    currStt = ADDR;
                    break;
                }
                else
                {
                    printf("[ERROR]@valid\tStart Byte does not contain Flag\n");
                    return 0;
                }
            }
            case (ADDR):
            {
                if (data[i] == ADDRS)
                {
                    printf("[SUCCESS]@valid\tAddress field is validated\n");
                    currStt = CTRLL;
                    break;
                }
                else
                {
                    printf("[ERROR]@valid\tAddress field is not validated\n");
                    return 0;
                }
            }
            case (CTRLL):
            {
                if (g_ctrl.currPar == 0)
                {
                    if (data[i] == CTR_PAR0)
                    {
                        printf("[SUCCESS]@valid\tControll field is validated\n");
                        currStt = BCC;
                        break;
                    }
                    else if (data[i] == CTR_PAR1)
                    {
                        printf("[ERROR]@valid\tAddress field is not validated, has incorrect parity\n");
                        return 0;
                    }
                    else
                    {
                        printf("[ERROR]@valid\tAddress field is not validated, unrecognized value\n");
                        return 0;
                    }
                }
                else if (g_ctrl.currPar == 1)
                {
                    if (data[i] == CTR_PAR0)
                    {
                        printf("[ERROR]@valid\tAddress field is not validated, has incorrect parity\n");
                        return 0;
                    }
                    else if (data[i] == CTR_PAR1)
                    {
                        printf("[SUCCESS]@valid\tControll field is validated\n");
                        currStt = BCC;
                        break;
                    }
                    else
                    {
                        printf("[ERROR]@valid\tAddress field is not validated, unrecognized value\n");
                        return 0;
                    }
                }
                else
                {
                    printf("[ERROR]@valid\tInternal Receiver Parity Issue\n");
                    return 0;
                }
            }
            case (BCC):
            {
                if (data[i] == (data[i - 1] ^ data[i - 2]))
                {
                    printf("[SUCCESS]@valid\tBCC field is validated\n");
                    currStt = BCC2;
                    break;
                }
                else
                {
                    printf("[ERROR]@valid\tBCC field is not validated\n");
                    return 0;
                }
            }
            case (BCC2):
            {
                unsigned int sz = sizeOf;

                unsigned char *buff = extractDataFromFrame(data, &sz);
                if (buff == NULL)
                {
                    //error
                    return 0;
                }
                unsigned char *bf = destuffData(buff, &sz);
                if (bf == NULL)
                {
                    //error
                    return 0;
                }

                unsigned char b2 = calculateBCC2(bf, sz);

                if (data[sizeOf - 2] == b2)
                {
                    printf("[SUCCESS]@valid\tBCC2 field is validated\n");
                    currStt = FLAG_END;
                    break;
                }
                else
                {
                    printf("[ERROR]@valid\tBCC2 field is not validated\n");
                    return 0;
                }
            }
            case (FLAG_END):
            {
                if (data[sizeOf - 1] == FLAG)
                {
                    printf("[SUCCESS]@valid\tEnd Flag field is validated\n");
                    currStt = DONE_PROC;
                    break;
                }
                else
                {
                    printf("[ERROR]@valid\tEnd Flag is not validated\n");
                    return 0;
                }
            }
            }
            if (currStt == DONE_PROC)
            {
                if (g_ctrl.currPar == 0)
                {
                    printf("[LOG]@valid\tFinished Frame Validation for information frame\n");
                    return 2;
                }
                else if (g_ctrl.currPar == 1)
                {
                    printf("[LOG]@valid\tFinished Frame Validation for information frame\n");
                    return 2;
                }
                else
                {
                    printf("[ERROR]@valid\tInternal Receiver Parity Issue\n");
                    return 0;
                }
                break;
            }
        }

        printf("[LOG]@valid\tFinished Frame Validation for information frame, should not have reacher here\n");
        return 0;
    }
    printf("[LOG]@valid\tFinished Frame Validation for information frame, should not have reacher here\n");
    return 0;
}

void setHandler()
{
    printf("[LOG]@setHandler\tSetting controll structure value\n");
    g_ctrl.currPar = 0;
    g_ctrl.retryCounter = 0;
    g_ctrl.hasTimedOut = FALSE;
    g_ctrl.shouldDC = FALSE;
    printf("[LOG]@setHandler\tDone Setting controll structure value\n");
}

void discHandler()
{
    printf("[LOG]@discHandler\tSetting controll value for disconnedtion\n");
    g_ctrl.shouldDC = TRUE;
    printf("[LOG]@discHandler\tDone Setting controll value for disconnedtion\n");
}
