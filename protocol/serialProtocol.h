#pragma once

#ifndef _SERIAL_PROTOCOL
#define _SERIAL_PROTOCOL

#define MAX_FRAME_SIZE 100
#define MAX_DATA_PER_FRAME 47 //only 94 bytes of information can be sent and if all bytes of the message need to be sutted then 
							  // it will go over the limit , to that degree we use 47 such that if all bytes need stuffing we will
							  // be using 94 bytes for the stuffed data ensuring we do not go over the 100 byter per frame limit.

/*
	Level:Sender

	This struct is used to manage the frame sending protocol and dealing with the responsees from the receptor
*/
struct SEND_CONTROLL{
	unsigned char** frames;
	unsigned char* frameToSend;
	unsigned int sendFrameNumber;
	unsigned int currentlySendingNumber;
}

//  The level field indicates wheter a function is used on the sender, the receptor or both

/*
    Level:Sender

	This function allocates the memory required to create and transfer the data frames, and then return the number of frames that were
	created.

	@Param buff - buffer to store the frames
	@Param data - data to be transfered, used to calculate the number of frames required

	@return - the number of frames allocated
*/
unsigned int allocateInformationFrames(unsigned char** buff,const unsigned char data[]);

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
unsigned char** divideData(const unsigned char data[]);

/*
	Level:Sender

	This Function moves a given section of the total data into the frame for transfer it also sets the error checker (BCC2)
	after the move, this move is done with memmove as such should be safe.

	@param frame - the frame the data is going to be moved to
	@param data - the data to extract from
	@param dataSection - the section from the data that should be moved into the frame
	@param numberOfSections - the number of sections the suplied data is divided into
*/
void moveInformationToFrame(unsigned char* frame,const unsigned char data[],unsigned int dataSection,unsigned int numberOfSections);

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
unsigned char calculateBCC2(const unsigned char data[]);

/*
	Level:Both

	This Function open the serial port and return its file descriptor

	@param serialPort - the serial port you want to connect to
	@param flags - the flags you want to use to open the file , if 0 is used the default values will be used

	@return the file descriptor of the serial port specified
*/
unsigned int openConnection(char* serialPort,unsgined unsigned int flags = 0);

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
unsigned char[] byteStuffingOnData(const unsigned char data[],unsigned int* sizeOfData);

/*
	Level:Sender

	This function interprets a response sent by the receptor and returns a value acordingly.To do this it uses a simple state machine

	@param receptorResponse - the frame with the receptor response

	@return wich type of responde the receptor sent:
				1 - Positive ACK
				0 - Unordered ACK
				-1 - Rejection ACK
*/
unsigned char ReceptorResponseInterpreter(const unsigned char* receptorResponse);

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

	@return - 1 in case of success , 0 in case an error occurred.
*/
unsigned char sendData(unsigned int fd,const unsigned char data[]);

#endif