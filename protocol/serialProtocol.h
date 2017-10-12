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

void moveInformationToFrame(unsigned int* frame,unsigned int data[],unsigned int dataSection,unsigned int numberOfSections);

void prepareInformationFrames(unsigned int** frames,unsigned int data[],unsigned int numberFrames)

#endif