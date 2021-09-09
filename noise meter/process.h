#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "buff.h"
#include "values.h"
#include <stdio.h>


#define pref 2*pow(10,-5)//valor de pressão de referencia
#define MARGIN pow(10,-10)//margem de -100dB

#define CALIBRATION_TIME 30//tempo de calibração

typedef struct{
	float *Prms;
	int num_elems;
	float calcedValue;//valor calculado para o calibrador
	float definedValue;//valor correspondente real 94dB
	
}Calibrator;

Calibrator *initCalibrator(float val);//inicializar o calibrador
void addValueToCalibrator(Calibrator *cal, float prms);//adição de valor para o calibrador
void calcCalibrator(Buffer *buff, int size,Calibrator *cal);//rms medido num bloco
void calcValueCalibrator(Calibrator *cal);//rms calculado no final
float calibrate(Calibrator *cal, float value);//calibrar e passar para dB
float decalibrate(Calibrator *cal, float calibrated);//change calibrated value to decalibritaded
float toLinear(float val);

void square(Buffer *buff,int size);//quadrado
void calcPeak(Buffer *buff,int size,Values_struct *values);//cálculo do peak
void processBuff(Buffer *buff,Values_struct *value, int size);//medições lae,lmax,lmin
void calcFinal(Values_struct *values, Calibrator *cal);//cálculo de todas as medições apos 1 segundo
	

#endif
