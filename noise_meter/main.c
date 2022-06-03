#include <stdio.h>
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <time.h>
#include <jansson.h>
#include "buff.h"
#include "values.h"
#include "process.h"
#include "filter.h"
#include "client.h"


void writeToFile(char*json, char *fileName) {//escrever o json formado num ficheiro
	FILE *file = fopen(fileName, "w");
	int results = fputs(json, file);
	if (results == EOF) {
		printf("Erro na escrita\n");
		exit(0);
	}
	fclose(file);
}

snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

int main (int argc, char *argv[]) {	// ./main default STATION_x EVENT_NAME
	snd_pcm_t *capture_handle;
	
	int result = snd_pcm_open(&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0);
	if (result < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n",
			 argv[1],
			 snd_strerror (result));
			 exit(EXIT_FAILURE);
	}
	result = snd_pcm_set_params(capture_handle,
					format,
					SND_PCM_ACCESS_RW_INTERLEAVED,
					1,
					WaveRate,
					1,
					500000);   /* 0.5 sec */
	if (result < 0) {
		fprintf(stderr, "snd_pcm_set_params: %s\n", snd_strerror(result));
		exit(EXIT_FAILURE);
	}

	result = snd_pcm_prepare(capture_handle);
	if (result < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
			 snd_strerror(result));
		exit(EXIT_FAILURE);
	}
	
	int seconds = 0;	//Measure hown many seconds have passed during calibration
	Buffer *calibrator_buff = createBuffers();	//Create buffer
	Calibrator *cal = initCalibrator(94);		//inicializar calibrador com 94dB de valor de referencia
	float coef_a[] = {
		1.00000000000000000,
		-2.12979364760736134,
		0.42996125885751674,
		1.62132698199721426,
		-0.96669962900852902,
		0.00121015844426781,
		0.04400300696788968};
	float coef_b[] = {
		0.169994948147430,
		0.280415310498794,
		-1.120574766348363,
		0.131562559965936,
		0.974153561246036,
		-0.282740857326553,
		-0.152810756202003};
	TimeWeight_struct *tw = initTimeWeightFILTER();		//Init Time weight filter
	Afilter_struct *af = initAFILTER(coef_a, coef_b, 6);	//inicializar estrutura com coeficientes
	int step = BLK_SIZE;	//blk
	
//-------------------------------CALIBRAÇÃO-------------------------------------------------	
	printf("Starting Calibration in %d seconds ...\n", CALIBRATION_TIME);
	while (1) {
		if (seconds == CALIBRATION_TIME) {
			printf("Calculating calibrated value...\n");
			calcValueCalibrator(cal);
			break;
		}
		
		// check if buffer can receive the new BLK_SIZE block
		if ((calibrator_buff->count + BLK_SIZE) > (WaveRate * SEGMENT_DURATION)) {
			step = (WaveRate * SEGMENT_DURATION) - calibrator_buff->count;	// Adjust the block the buffer receives	
		} else {
			step = BLK_SIZE;
		}
		// leitura de amostras
		snd_pcm_sframes_t read_frames = snd_pcm_readi(capture_handle, calibrator_buff->buff_16, step);
		if (read_frames < 0) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
					snd_strerror(read_frames));
			exit(EXIT_FAILURE);
		}
		//printf("buff16 = %p , step = %d \n",calibrator_buff->buff_16,step);
		
		if (seconds >= (CALIBRATION_TIME - 5)) {	// ultimos 5 segundos para calibração
			bufferInternalCopy(calibrator_buff, step);
			AfilterFILTERING(calibrator_buff, step, af);
			square(calibrator_buff, step);
			timeWeightFILTERING(calibrator_buff,step,tw);
			calcCalibrator(calibrator_buff,step,cal);
		}
		calibrator_buff->count = calibrator_buff->count + step;	// Number of elements in buff
		calibrator_buff->buff = calibrator_buff->buff + step;	// Change the pointer location
		calibrator_buff->buff_16 = calibrator_buff->buff_16 + step;
		
		if (calibrator_buff->count == (WaveRate * SEGMENT_DURATION)) {	// If 1 second has passed
			resetBuff(calibrator_buff);	
			seconds = seconds + 1;
			printf("%d\n", seconds);
		}
	}
	
	//--------------------------------------------------------------------------------	
	printf("%lf <-> %lf\n", cal->definedValue, cal->calcedValue);
	
	//-----Connect to Python Server----	
	Client_Struct *cli = initClient();
	int socket_desc = 0;
	struct sockaddr_in server;
	connectSocket(cli, socket_desc, server);
	
	//----------------------------------

	char *STATION_ID = argv[2];	//Station identifier
	char *EVENT_NAME = argv[3];	//Event name
	
	fflush(stdout);
	
	Values_struct *values = createValue();	// create value struct
	Buffer *buff_1 = createBuffers();		// Create buffer
	
	//-----Create Json tree---------------------------
	json_t *evento = json_object();
	json_t *id = json_object();
	json_t *levels = json_object();
	json_t *root = json_object();
	json_t *timestamp = json_array();
	json_t *Lae = json_array();
	json_t *Lmax = json_array();
	json_t *Lmin = json_array();
	json_t *Lpeak = json_array();
	json_t *Laeq = json_array();
  
	json_object_set_new( root, "LAmax", Lmax);
	json_object_set_new( root, "LAmin", Lmin );
	json_object_set_new( root, "LApeak", Lpeak );
	json_object_set_new( root, "LAE", Lae );
	json_object_set_new( root, "LAEQ", Laeq );
	json_object_set_new( root, "timestamps", timestamp );
	json_object_set_new( levels, "noise_levels", root );
	json_object_set_new( id, STATION_ID, levels );
	json_object_set_new( evento, EVENT_NAME, id );

	//-----------------------------------------------------------	
	int window_step = BLK_SIZE;
	
	printf("Starting noise level measuring...\n");

	while (1) {
		if ( !checkBuff(buff_1)) {	// check if buffer can receive the new BLK_SIZE block
			window_step = (WaveRate * SEGMENT_DURATION) - buff_1->count;	// Adjust the block the buffer receives	
		} else {
			window_step = BLK_SIZE;
		}

		snd_pcm_sframes_t read_frames = snd_pcm_readi(capture_handle, buff_1->buff_16, window_step);
		if (read_frames < 0) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
					snd_strerror(read_frames));
			exit(EXIT_FAILURE);
		}		
		bufferInternalCopy(buff_1, window_step);	// Copy elements from the 16 bit array to the 32 bits
		
		AfilterFILTERING(buff_1, window_step, af);	// A-filtering
		square(buff_1, window_step);				// Square all thew values 
		calcPeak(buff_1, window_step, values);		// Calculate peak value
		
		timeWeightFILTERING(buff_1, window_step, tw);	// Time weight filtering
		processBuff(buff_1, values, window_step);		// Calculate lmax, lmin, lae
			
		buff_1->count = buff_1->count + window_step;	// Number of elements in buff
		buff_1->buff = buff_1->buff + window_step;		// Change the pointer location
		buff_1->buff_16 = buff_1->buff_16 + window_step;
		
		if (buff_1->count == (WaveRate * SEGMENT_DURATION)) {	// If buff is full
			calcFinal(values,cal);	// Calc every one second - Max from lmax, average from lae, min from lmin, etc...
			
			//Add values to the Json tree	
			json_array_append(Lae, json_integer(values->LAE_buff[values->count-1]));
			json_array_append(Lmax, json_integer(values->LAFmax_buff[values->count-1]));
			json_array_append(Lmin, json_integer(values->LAFmin_buff[values->count-1]));
			json_array_append(Laeq, json_integer(values->LAeq[values->count-1]));
			json_array_append(Lpeak, json_integer(values->LApeak_buff[values->count-1]));
			json_array_append(timestamp, json_integer((int)time(NULL)));

			values->idx = values->num_elem;	// Identify the idx of the beggining of each second	
			resetBuff(buff_1);		// Reset the buffer for the next second	
		}
		
		if (values->count == (cli->send_counter * SEND_TIME_S)) {
			writeToFile(json_dumps(evento,0),"Log.json");	// Storage
			sendJson(cli,json_dumps(evento,0));		// Send Json tree to python server
			cli->send_counter = cli->send_counter + 1;	// Increment the counter
		}	
	}
	snd_pcm_close (capture_handle);
}