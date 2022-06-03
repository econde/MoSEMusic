#ifndef _BUFF_H_
#define _BUFF_H_

#include <stdlib.h>
#include <stdbool.h>

#define SEGMENT_DURATION 1	// duration of each total segment - 1 second
#define WaveRate 48000		// sample rate
#define BLK_SIZE 1024		// default size of each block

typedef struct {
	int count;	// preenchidos
	short *buff_16;	// utilizado apenas para guardar as amostras inicialmente
	float *buff; 
} Buffer; 

Buffer *createBuffers( void );	// Cria o buffer
bool checkBuff(Buffer *buff);	// Verifica se o buffer consegue receber mais um BLK
void bufferInternalCopy(Buffer *buff, int size);	//copiar os dados de 16 bits para o buffer de 32 bits
void resetBuff(Buffer *buff);	// Resetar o ponteiro do bufffer e a sua dimensão

#endif
