#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <complex.h>
#include <stdbool.h>


// Include the ALSA .H file that defines ALSA functions/data
#include <alsa/asoundlib.h>
#include <math.h>

#include "make_wav.h" 

#define BLK_SIZE 1024

#pragma pack (1)



/////////////////////// WAVE File Stuff /////////////////////
// An IFF file header looks like this
typedef struct _FILE_head
{
	unsigned char	ID[4];	// could be {'R', 'I', 'F', 'F'} or {'F', 'O', 'R', 'M'}
	unsigned int	Length;	// Length of subsequent file (including remainder of header). This is in
									// Intel reverse byte order if RIFF, Motorola format if FORM.
	unsigned char	Type[4];	// {'W', 'A', 'V', 'E'} or {'A', 'I', 'F', 'F'}
} FILE_head;


// An IFF chunk header looks like this
typedef struct _CHUNK_head
{
	unsigned char ID[4];	// 4 ascii chars that is the chunk ID
	unsigned int	Length;	// Length of subsequent data within this chunk. This is in Intel reverse byte
							// order if RIFF, Motorola format if FORM. Note: this doesn't include any
							// extra byte needed to pad the chunk out to an even size.
} CHUNK_head;

// WAVE fmt chunk
typedef struct _FORMAT {
	short				wFormatTag;
	unsigned short	wChannels;
	unsigned int	dwSamplesPerSec;
	unsigned int	dwAvgBytesPerSec;
	unsigned short	wBlockAlign;
	unsigned short	wBitsPerSample;
  // Note: there may be additional fields here, depending upon wFormatTag
} FORMAT;
#pragma pack()



#define DURATION 5


// Size of the audio card hardware buffer. Here we want it
// set to 1024 16-bit sample points. This is relatively
// small in order to minimize latency. If you have trouble
// with underruns, you may need to increase this, and PERIODSIZE
// (trading off lower latency for more stability)
#define BUFFERSIZE	(2*1024)

// How many sample points the ALSA card plays before it calls
// our callback to fill some more of the audio card's hardware
// buffer. Here we want ALSA to call our callback after every
// 64 sample points have been played
#define PERIODSIZE	(2*64)

// Handle to ALSA (audio card's) playback port
snd_pcm_t				*PlaybackHandle;

// Handle to our callback thread
snd_async_handler_t	* CallbackHandle;

// Points to loaded WAVE file's data
short		*WavePtr;

// Size (in frames) of loaded WAVE file's data
snd_pcm_uframes_t		WaveSize;

// Sample rate
unsigned short			WaveRate;

// Bit resolution
unsigned char			WaveBits;

// Number of channels in the wave file
unsigned char			WaveChannels;

// The name of the ALSA port we output to. In this case, we're
// directly writing to hardware card 0,0 (ie, first set of audio
// outputs on the first audio card)
//static const char		SoundCardPortName[] = "plughw:1,1";


// For WAVE file loading
static const unsigned char Riff[4]	= { 'R', 'I', 'F', 'F' };
static const unsigned char WaveF[4] = { 'W', 'A', 'V', 'E' };
static const unsigned char Fmt[4] = { 'f', 'm', 't', ' ' };
static const unsigned char Data[4] = { 'd', 'a', 't', 'a' };




static unsigned char compareID(const unsigned char * id, unsigned char * ptr){
	register unsigned char i = 4;

	while (i--)
	{
		if ( *(id)++ != *(ptr)++ ) return(0);
	}
	return(1);
}


// -------------------------------------------------- [ Section: Endianness ] -
int isBigEndian() {
    int test = 1;
    char *p = (char*)&test;

    return p[0] == 0;
}
void reverseEndianness(const long long int size, void* value){
    int i;
    char result[32];
    for( i=0; i<size; i+=1 ){
        result[i] = ((char*)value)[size-i-1];
    }
    for( i=0; i<size; i+=1 ){
        ((char*)value)[i] = result[i];
    }
}
void toBigEndian(const long long int size, void* value){
    char needsFix = !( (1 && isBigEndian()) || (0 && !isBigEndian()) );
    if( needsFix ){
        reverseEndianness(size,value);
    }
}
void toLittleEndian(const long long int size, void* value){
    char needsFix = !( (0 && isBigEndian()) || (1 && !isBigEndian()) );
    if( needsFix ){
        reverseEndianness(size,value);
    }
}



WaveHeader makeWaveHeader(int const sampleRate, short int const numChannels, short int const bitsPerSample ){
    WaveHeader myHeader;

    // RIFF WAVE Header
    myHeader.chunkId[0] = 'R';
    myHeader.chunkId[1] = 'I';
    myHeader.chunkId[2] = 'F';
    myHeader.chunkId[3] = 'F';
    myHeader.format[0] = 'W';
    myHeader.format[1] = 'A';
    myHeader.format[2] = 'V';
    myHeader.format[3] = 'E';

    // Format subchunk
    myHeader.subChunk1Id[0] = 'f';
    myHeader.subChunk1Id[1] = 'm';
    myHeader.subChunk1Id[2] = 't';
    myHeader.subChunk1Id[3] = ' ';
    myHeader.audioFormat = 1; // FOR PCM
    myHeader.numChannels = numChannels; // 1 for MONO, 2 for stereo
    myHeader.sampleRate = sampleRate; // ie 44100 hertz, cd quality audio
    myHeader.bitsPerSample = bitsPerSample; // 
    myHeader.byteRate = myHeader.sampleRate * myHeader.numChannels * myHeader.bitsPerSample / 8;
    myHeader.blockAlign = myHeader.numChannels * myHeader.bitsPerSample/8;

    // Data subchunk
    myHeader.subChunk2Id[0] = 'd';
    myHeader.subChunk2Id[1] = 'a';
    myHeader.subChunk2Id[2] = 't';
    myHeader.subChunk2Id[3] = 'a';

    // All sizes for later:
    // chuckSize = 4 + (8 + subChunk1Size) + (8 + subChubk2Size)
    // subChunk1Size is constanst, i'm using 16 and staying with PCM
    // subChunk2Size = nSamples * nChannels * bitsPerSample/8
    // Whenever a sample is added:
    //    chunkSize += (nChannels * bitsPerSample/8)
    //    subChunk2Size += (nChannels * bitsPerSample/8)
    myHeader.chunkSize = 4+8+16+8+0;
    myHeader.subChunk1Size = 16;
    myHeader.subChunk2Size = 0;
    
    return myHeader;
}

/********************** waveLoad() *********************
 * Loads a WAVE file.
 *
 * fn =			Filename to load.
 *
 * RETURNS: 0 if success, non-zero if not.
 *
 * NOTE: Sets the global "WavePtr" to an allocated buffer
 * containing the wave data, and "WaveSize" to the size
 * in sample points.
 */

Wave makeWave(int const sampleRate, short int const numChannels, short int const bitsPerSample){
    Wave myWave;
    myWave.header = makeWaveHeader(sampleRate,numChannels,bitsPerSample);
    return myWave;
}
void waveDestroy( Wave* wave ){
    free( wave->data );
}
void waveSetDuration( Wave* wave, const float seconds ){
    long long int totalBytes = (long long int)(wave->header.byteRate*seconds);
    wave->data = (char*)malloc(totalBytes);
    wave->index = 0;
    wave->size = totalBytes;
    wave->nSamples = (long long int) wave->header.numChannels * wave->header.sampleRate * seconds;
    wave->header.chunkSize = 4+8+16+8+totalBytes;
    wave->header.subChunk2Size = totalBytes;
}
void waveAddSample( Wave* wave, const double* samples ){
    int i;
    short int sample8bit;
    int sample16bit;
    long int sample32bit;
    char* sample;
    if( wave->header.bitsPerSample == 8 ){
        for( i=0; i<wave->header.numChannels; i+= 1){
            sample8bit = (short int) (127+127.0*samples[i]);
            toLittleEndian(1, (void*) &sample8bit);
            sample = (char*)&sample8bit;
            (wave->data)[ wave->index ] = sample[0];
            wave->index += 1;
        }
    }
    if( wave->header.bitsPerSample == 16 ){
        for( i=0; i<wave->header.numChannels; i+= 1){
            sample16bit = (int) (32767*samples[i]);
            //sample = (char*)&litEndianInt( sample16bit );
            toLittleEndian(2, (void*) &sample16bit);
            sample = (char*)&sample16bit;
            wave->data[ wave->index + 0 ] = sample[0];
            wave->data[ wave->index + 1 ] = sample[1];
            wave->index += 2;
        }
    }
    if( wave->header.bitsPerSample == 32 ){
        for( i=0; i<wave->header.numChannels; i+= 1){
            sample32bit = (long int) ((pow(2,32-1)-1)*samples[i]);
            //sample = (char*)&litEndianLong( sample32bit );
            toLittleEndian(4, (void*) &sample32bit);
            sample = (char*)&sample32bit;
            wave->data[ wave->index + 0 ] = sample[0];
            wave->data[ wave->index + 1 ] = sample[1];
            wave->data[ wave->index + 2 ] = sample[2];
            wave->data[ wave->index + 3 ] = sample[3];
            wave->index += 4;
        }
    }
}
void waveToFile( Wave* wave, const char* filename ){

    // First make sure all numbers are little endian
    toLittleEndian(sizeof(int), (void*)&(wave->header.chunkSize));
    toLittleEndian(sizeof(int), (void*)&(wave->header.subChunk1Size));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.audioFormat));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.numChannels));
    toLittleEndian(sizeof(int), (void*)&(wave->header.sampleRate));
    toLittleEndian(sizeof(int), (void*)&(wave->header.byteRate));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.blockAlign));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.bitsPerSample));
    toLittleEndian(sizeof(int), (void*)&(wave->header.subChunk2Size));

    // Open the file, write header, write data
    FILE *file;
    file = fopen(filename, "wb");
    fwrite( &(wave->header), sizeof(WaveHeader), 1, file );
    fwrite( (void*)(wave->data), sizeof(char), wave->size, file );
    fclose( file );

    // Convert back to system endian-ness
    toLittleEndian(sizeof(int), (void*)&(wave->header.chunkSize));
    toLittleEndian(sizeof(int), (void*)&(wave->header.subChunk1Size));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.audioFormat));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.numChannels));
    toLittleEndian(sizeof(int), (void*)&(wave->header.sampleRate));
    toLittleEndian(sizeof(int), (void*)&(wave->header.byteRate));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.blockAlign));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.bitsPerSample));
    toLittleEndian(sizeof(int), (void*)&(wave->header.subChunk2Size));
}

static unsigned char waveLoad(const char *fn) {
	const char				*message;
	FILE_head				head;
	register int			inHandle;

	if ((inHandle = open(fn, O_RDONLY)) == -1)
		message = "didn't open";

	// Read in IFF File header
	else
	{
		if (read(inHandle, &head, sizeof(FILE_head)) == sizeof(FILE_head))
		{
			// Is it a RIFF and WAVE?
			if (!compareID(&Riff[0], &head.ID[0]) || !compareID(&WaveF[0], &head.Type[0]))
			{
				message = "is not a WAVE file";
				goto bad;
			}

			// Read in next chunk header
			while (read(inHandle, &head, sizeof(CHUNK_head)) == sizeof(CHUNK_head))
			{
				// ============================ Is it a fmt chunk? ===============================
				if (compareID(&Fmt[0], &head.ID[0]))
				{
					FORMAT	format;

					// Read in the remainder of chunk
					if (read(inHandle, &format.wFormatTag, sizeof(FORMAT)) != sizeof(FORMAT)) break;

					// Can't handle compressed WAVE files
					if (format.wFormatTag != 1)
					{
						message = "compressed WAVE not supported";
						goto bad;
					}
					
				

					WaveBits = (unsigned char)format.wBitsPerSample;
					WaveRate = (unsigned short)format.dwSamplesPerSec;
					WaveChannels = format.wChannels;
					
				}
				

				// ============================ Is it a data chunk? ===============================
				else if (compareID(&Data[0], &head.ID[0]))
				{
					// Size of wave data is head.Length. Allocate a buffer and read in the wave data
					if (!(WavePtr = (short *)malloc(head.Length)))
					{
						message = "won't fit in RAM";
						goto bad;
					}
				
					
					if (read(inHandle, WavePtr, head.Length) != head.Length)
					{
						free(WavePtr);
						break;
					}
					// Store size (in frames)
					
					WaveSize = (head.Length * 8) / ((unsigned int)WaveBits * (unsigned int)WaveChannels);

					close(inHandle);
					
					return(0);
				}

				// ============================ Skip this chunk ===============================
				else
				{
					if (head.Length & 1) ++head.Length;  // If odd, round it up to account for pad byte
					lseek(inHandle, head.Length, SEEK_CUR);
				}
				
			}
			
		}

		message = "is a bad WAVE file";
		bad:	close(inHandle);
	}
	
	printf("%s %s\n", fn, message);
	return(1);
}

void shift_right(float* u, int size,float x) {
    
   memmove(&u[1],&u[0],size *sizeof(float));
   u[0]=x;

}


void AFilter(short *x,int norm,double *y, int size, int N, float*a,float*b,float *u){
	int u_size = N;
	
	for(int n = 0; n<size;n++){
		
		float normal = (float)x[n]/norm;
		shift_right(u,u_size,normal);//shifta para o lado e u[n] = x[n] => u[0] = x[n]
		
		for(int i = 1;i<=N;i++){
			u[0] = u[0] + (((-1)*a[i])*u[i]);//u(n) = u(n) + (-1)a(i)*u(n-i)
		}
		for(int i = 0; i<=N;i++){
			y[n] = y[n] + (b[i]*u[i]);//y(n) = y(n)+b(i)*u(n-i)
			
		}
		
		//printf("%lf ",y[n]);
	}
		
}

typedef struct{
	int count;//preenchidos
	short *buff; 
} Buffer; 
//Creates a Buffer element
//Initialize buff with size of 1 second
Buffer *createBuffers( void ){
	Buffer * create=malloc(sizeof*create);
	create->count = 0;
	create-> buff = malloc((WaveRate) * sizeof(short));
	return create;
}

//Checks if buffer can receive another block of BLK_SIZE
//Returns True if it can
//Returns False if it cant
bool checkBuff(Buffer *buff){
	return (buff->count + BLK_SIZE) <= (WaveRate);
}

//Resets the buffer
//Empty number of elements
//Reset the pointer back to the beginning
void resetBuff(Buffer *buff){
	buff->buff = buff->buff - (WaveRate);
	buff->count = 0;
}


int main(){
	
	
	int norm = pow(2,(sizeof (short)*8)-1);
	waveLoad("WhiteNoise.wav");
	
	int N = 6;//Filter order
	

	int size = WaveSize;
	//Coefficients
	float b[] = {0.169994948147430,0.280415310498794,-1.120574766348363,0.131562559965936,0.974153561246036,-0.282740857326553,-0.152810756202003};
	float a[] = {1.00000000000000000,-2.12979364760736134,0.42996125885751674,1.62132698199721426,-0.96669962900852902,0.00121015844426781,0.04400300696788968};
	
	
	double *y = calloc(size, sizeof(double));//y=0
	float *u = calloc(N+1, sizeof(float));
	
	int i =0;//Used to know when the loop is over
	int window_step = BLK_SIZE;
	Wave w = makeWave(WaveRate,WaveChannels,WaveBits);
	waveSetDuration(&w,1);
	
	Buffer *buff_1 = createBuffers();//Create buffer
	
		
	while(i<WaveSize){
		
		if(!checkBuff(buff_1)){//check if buffer can receive the new BLK_SIZE block
			window_step = (WaveRate ) - buff_1->count;//Adjust the block the buffer receives
		}else{
			window_step = BLK_SIZE;
		}
			
		memcpy(buff_1->buff,WavePtr,window_step * sizeof(short));//read biblioteca alsa
		
		AFilter(WavePtr,norm,y,window_step,N,a,b,u);
		
		for(int i=0; i<window_step; i++){
		
			waveAddSample( &w, &y[i] );
		
		}
		y = y + window_step;
			
		
		buff_1->count = buff_1->count + window_step;//Number of elements in buff
		buff_1->buff = buff_1->buff + window_step;//Change the pointer location
		
		WavePtr = WavePtr + window_step;//Change the pointer location
		
		if(buff_1->count == (WaveRate)){//If buff is full	
			resetBuff(buff_1);
		}
			 
			
			i = i+window_step;
		
	}
	
	
	
	
	
	
	waveToFile( &w, "WhiteNoiseA.wav");
    waveDestroy( &w );
 
	return 0;
}

