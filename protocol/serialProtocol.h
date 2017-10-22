#pragma once

#ifndef _SERIAL_PROTOCOL
#define _SERIAL_PROTOCOL

#define FLAG 0x7E
#define ADDRS 0x03
#define SET 0x03
#define ADDR2 0x01
#define DISC 0x0B

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define TIMEOUT 3
#define MAX_TIMEOUT 3

#define FLAG_STR 0
#define ADDR 1
#define CTRL 2
#define BCC 3
#define FLAG_END 4
#define DONE_PROC 5

/*
	Response types calculated for each of the 2 possible frame parities, 0 or 1
*/
#define REJ1 0x81
#define REJ0 0x01
#define RR0 0x05
#define RR1 0x85
#define UA 0x07

#define UA_R 0
#define REJ -1
#define ACPT 1
#define ERR -2

#define MAX_FRAME_SIZE 206
#define MAX_DATA_PER_FRAME 100 //only 94 bytes of information can be sent and if all bytes of the message need to be sutted then 
							   // it will go over the limit , to that degree we use 47 such that if all bytes need stuffing we will
							   // be using 94 bytes for the stuffed data ensuring we do not go over the 100 byter per frame limit.

struct termios oldtio,newtio; //structures with the information about the serial port pipe connection

/*
	Level:Sender

	This struct is used to manage the frame sending protocol and dealing with the responsees from the receptor
*/
struct SEND_CONTROLL{
	unsigned char* frameToSend;
	unsigned char currPar;
	unsigned char retryCounter;
	unsigned char hasTimedOut;
	unsigned int fileDescriptor;
	unsigned int lastFrameSize;
	unsigned int allocError;
};

struct SEND_CONTROLL g_ctrl;//global controll structure for the protocol to use

//  The level field indicates wheter a function is used on the sender, the receptor or both

/*
    Level:Sender

	This function allocates the memory required to create and transfer the data frames, and then return the number of frames that were
	created.

	@Param buff - buffer to store the frames
	@Param data - data to be transfered, used to calculate the number of frames required

	@return - the number of frames allocated
*/
unsigned int allocateInformationFrames(unsigned char** buff,const unsigned char data[],unsigned int sizeOf);

/*
    Level:Sender

	This Function frees the memory allocated for the frames and the memory allocated for the frame buffer

	@Param frames - the buffer where the frames are being kept
    
    @Param numberOfFrames - the number of frames inside the frame buffer , this value is returned by the prepareData function
*/
void deallocateInformationFrames(unsigned char** frames,unsigned int numberOfFrames);

/*
	Level:Sender

	This function splits the data into chunks apropriate to send through the data frames

	@param data - the data that must be divided

	@return a buffer with the divided data chunks
*/
unsigned char** divideData(const unsigned char data[],unsigned int* sizeOf);

/*
	Level:Sender

	This Function moves a given section of the total data into the frame for transfer it also sets the error checker (BCC2)
	after the move, this move is done with memmove as such should be safe.

	@param frame - the frame the data is going to be moved to
	@param data - the data to move to the frame,this data must at most be of size MAX_DATA_PER_FRAME
*/
void moveInformationToFrame(unsigned char* frame,const unsigned char data[],unsigned int size);

/*
	Level:Sender

	This function moves the supplied data into the network frames for sending,before executing the move it
	splits the data and executes the byte stuffing functions as specified by the protocol on the data ,
	 as such we must guarante that the frame can hold 2x the size of the data supplied here.

	@param frames - the frames to move the data into
	@param data - the data that needs to be moved into the frames
	@param size - the size in bytes of the data supplied
	@parm numberOfFrames - the number of frames in the frame buffer
*/
void moveDataToFrames(unsigned char** frames,const unsigned char data[],unsigned int size,unsigned int numberOfFrames);

/*
	Level:Sender

	This function prepares a information transfer frame, it sets its flag bytes, the address byte, the command byte and
	the error checking block 1(BCC1)

	@param frames - the buffer of frames that need to be prepared
	@param numberFrames - the ammount of frames in the buffer that should be prepared
*/
void prepareInformationFrames(unsigned char** frames,unsigned int numberFrames);

/*
	Level:Both

	This function calculates the error checking block for a given array of data

	@param data - the piece of data to calculate the BCC on

	@return the error checking block (BCC)
*/
unsigned char calculateBCC2(const unsigned char data[],unsigned int sizeOfData);

/*
	Level:Both

	This Function open the serial port and return its file descriptor

	@param serialPort - the serial port you want to connect to
	@param flags - the flags you want to use to open the file , if 0 is used the default values will be used

	@return the file descriptor of the serial port specified
*/
unsigned int openConnection(char* serialPort,unsigned int flags);

/*
	Level:Both

	This function closes the connection to the specified file descriptor

	@param fd - file descriptor of the serial port you want to disconect from
*/
void closeConnection(unsigned int fd);

/*
	Level:Sender

	This functions is responsible for byte stuffing the data in order for proper transfer over the serial port.
	to do this is substitutes any byte with the value 0x7E(the frame flag) with the bytes 0x7D 0x5D (0x7E XOR 0x20)

	@param data - the data that needs to be stuffed
	@param sizeOfData - the initial size in bytes of the data, this value will be substituted with the size of the stuffed data

	@return an array the data in its stuffed format
*/
unsigned char* byteStuffingOnData(const unsigned char data[],unsigned int* sizeOfData);

/*
	Level:Sender

	This function interprets a response sent by the receptor and returns a value acordingly.To do this it uses a simple state machine

	@param receptorResponse - the frame with the receptor response

	@return wich type of responde the receptor sent:
				1 - Positive ACK
				0 - Unordered ACK
				-1 - Rejection ACK
*/
char ReceptorResponseInterpreter(const unsigned char* receptorResponse);

/*
	Level:Sender

	This Function sends a SET command frame to the receptor

	@param fd - the file descriptor of the receptor

	@return - 1 in case of success , 0 in case an error occurred.
*/
unsigned char sendSetCommand(unsigned int fd);

/*
	Level:Sender

	This Function sends a DISC command frame to the receptor

	@param fd - the file descriptor of the receptor

	@return - 1 in case of success , 0 in case an error occurred.
*/
unsigned char sendDisconnectCommand(unsigned int fd);

/*
	Level:Sender

	This Function sends data to the receptor, it internaly manages the division of
	the data frames and waiting for proper response from the receptor

	@param fd - the file descriptor of the receptor
	@param data - data to be sent

	@return - ammount sent in case of success , 0 in case an error occurred.
*/
unsigned char sendData(unsigned int fd,const unsigned char data[],unsigned int size);

/*
	Level:Both

	This function allocated a number of generic unsinged char frame buffers

	@param numberOfBufefrs - the number of buffers that need to be allocated
	@param dataPerBuffer - the ammount of data that needs to be allocated per buffer

	@return - a set of dinamically allocated unsigned char buffers
*/
unsigned char** allocateCharBuffers(unsigned int numberOfBuffers,unsigned int dataPerBuffer);

/*
	Level:Both

	This function deallocates a generic unsigned char frame buffer

	@param buffers - The buffers that were allocated
	@param numberOfBuffers - The number of buffers that sould be deallocated
*/
void deallocatedCharBuffers(unsigned char** buffers,unsigned int numberOfBuffers);

/*
	Level:Sender

	This function is a handler for the SIGALRM signal , it is used when sending a frame over the network
	it has inbued retransmission to a specified ammount, marks in the g_ctrl structure that there has 
	been a timeout if such occurs

	@param sig - as per specified for this handler the ammount of time of the alarm.
*/
void timeoutHandler(int sig);

/*
	Level: Sender

	This function reads from the file descriptor the acknoledgement from the receiver,it also times out if that is
	needed.

	@param fd - file descriptor of the serial prot to read from

	@return - a dynamically allocated 5 byte buffer with the response from the receiver
*/
unsigned char* getReceptorResponse(unsigned int fd);

//DOCUMENTATION MISSING
unsigned char*destuffData(unsigned char* data,unsigned int *sizeOF);

#endif
