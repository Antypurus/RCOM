#pragma once

#ifndef _SERIAL_PROTOCOL
#define _SERIAL_PROTOCOL

#define MAX_FRAME_SIZE 100
#define MAX_DATA_PER_FRAME 94

//  The level field indicates wheter a function is used on the sender, the receptor or both

/*
    Level:Sender

	This function allocates the memory required to create and transfer the data frames, and then return the number of frames that were
	created.

	@Param buff - buffer to store the frames
	@Pram data - data to be transfered, used to calculate the number of frames required

	@return - the number of frames allocated
*/
unsigned int allocateInformationFrames(unsigned int** buff,unsigned int data[]);

/*
    Level:Sender

	This Function frees the memory allocated for the frames and the memory allocated for the frame buffer

	@Param frames - the buffer where the frames are being kept
    
    @Param numberOfFrames - the number of frames inside the frame buffer , this value is returned by the prepareData function
*/
void deallocateInformationFrames(unsigned int** frames,unsigned int numberOfFrames);

/*
	Level:Sender

	This Function moves a given section of the total data into the frame for transfer it also sets the error checker (BCC2)
	after the move, this move is done with memmove as such should be safe.

	@param frame - the frame the data is going to be moved to
	@param data - the data to extract from
	@param dataSection - the section from the data that should be moved into the frame
	@param numberOfSections - the number of sections the suplied data is divided into
*/
void moveInformationToFrame(unsigned int* frame,unsigned int data[],unsigned int dataSection,unsigned int numberOfSections);

/*
	Level:Sender

	This function prepares a information transfer frame, it sets its flag bytes, the address byte, the command byte and
	the error checking block 1(BCC1)

	@param frames - the buffer of frames that need to be prepared
	@param numberFrames - the ammount of frames in the buffer that should be prepared
*/
void prepareInformationFrames(unsigned int** frames,unsigned int numberFrames);

#endif