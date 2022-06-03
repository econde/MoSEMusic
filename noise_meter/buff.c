#include "buff.h"

//Creates a Buffer element
//Initialize buff with size of 1 second
Buffer *createBuffers( void ){
	Buffer *create = malloc(sizeof *create);
	if (create == NULL) 
		return NULL;
	create->count = 0;
	create->buff_16	= malloc((WaveRate * SEGMENT_DURATION) * sizeof(short));
	if (create->buff_16 == NULL) {
		free(create);
		return NULL;
	}
	create->buff = malloc((WaveRate * SEGMENT_DURATION) * sizeof(float));
	if (create->buff_16 == NULL) {
		free(create);
		free(create->buff_16);
		return NULL;
	}
	return create;
}

// Checks if buffer can receive another block of BLK_SIZE
// Returns True if it can
// Returns False if it cant
bool checkBuff(Buffer *buff) {
	return (buff->count + BLK_SIZE) <= (WaveRate * SEGMENT_DURATION);
}

void bufferInternalCopy(Buffer *buff, int size) {
	for (int i = 0; i < size; i++){
		buff->buff[i] = buff->buff_16[i];		
	}
}

// Resets the buffer
// Empty number of elements
// Reset the pointer back to the beginning
void resetBuff(Buffer *buff) {
	buff->buff = buff->buff - (int)(WaveRate * SEGMENT_DURATION);
	buff->buff_16 = buff->buff_16 - (int)(WaveRate * SEGMENT_DURATION);
	buff->count = 0;
}
