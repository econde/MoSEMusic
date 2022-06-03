#include "process.h"

Calibrator *initCalibrator(float val) {
	Calibrator *cal = malloc(sizeof *cal);
	cal->Prms = NULL;
	cal->num_elems = 0;
	cal->calcedValue = 0;
	cal->definedValue = val;
	return cal;
}
void addValueToCalibrator(Calibrator *cal, float prms){
	cal->Prms = realloc( cal->Prms, (cal->num_elems + 1) * sizeof *cal->Prms );	// adicionar 1 valor
	cal->Prms[cal->num_elems] = prms;
	cal->num_elems = cal->num_elems + 1;
}

void calcCalibrator(Buffer *buff, int size,Calibrator *cal){
	float sum = 0;
	for (int i = 0; i < size; i++){
		sum = sum + (buff->buff[i] + MARGIN);
	}
	float prms = sqrt(sum / size);	// calculo do rms por blk linear
	addValueToCalibrator(cal, prms);
}

void calcValueCalibrator(Calibrator *cal){
	float sum = 0;
	for (int i = 0; i < cal->num_elems; i++) {
		sum = sum + cal->Prms[i];
	}
	float val = sum / cal->num_elems;
	cal->calcedValue = 20 * log10(val / (float)(pref));	// calculo do valor do calibrado atraves da média dos rms para cada blk
}

float calibrate(Calibrator *cal, float lin) {	// calibrar um valor linear
	float log = 20 * log10(lin / (float)(pref));
	return (log + cal->definedValue - cal->calcedValue);
}

float decalibrate(Calibrator *cal, float calibrated) {	// descalibrar um valor - Utilizado para o LAEQ
	return (calibrated + cal->calcedValue - cal->definedValue);
}

float toLinear(float val){//passar valor logaritmico para linear
	return (pref * pow(10, val / 20));
}

void square(Buffer *buff, int size){
	for(int i = 0; i < size; i++){
		buff->buff[i] = pow(buff->buff[i],2);//quadrado
	}
}

void calcPeak(Buffer *buff, int size, Values_struct *values) {
	float peak = buff->buff[0];
	for (int i = 1; i < size; i++){
		if (peak < buff->buff[i] + MARGIN) {	// calcular o maximo
			peak = buff->buff[i] + MARGIN;
		}
		
	}
	peak = sqrt(peak);
	addPeak(values, peak);	// adicionar o peak
}

void processBuff(Buffer *buff, Values_struct *value, int size) {
	float sum = 0;
	float prms;
	float max = buff->buff[0];
	float min = buff->buff[0];
	for (int i = 0; i < size; i++) {
		if (max < buff->buff[i] + MARGIN) {	// calc max
			max = buff->buff[i] + MARGIN;
		}
		if (min > buff->buff[i] + MARGIN) {	// calc min
				min = buff->buff[i] + MARGIN;
		}	
		sum = sum + (buff->buff[i] + MARGIN);
	}
	max = sqrt(max);
	min = sqrt(min);
	prms = sqrt(sum / size);
	valueAdd(value, max, min, prms);	// adicionar o max, min e prms
}

void calcFinal(Values_struct *values, Calibrator *cal) {		
	float lae = 0;
	float max = values->LAFmin[values->idx];
	float min = values->LAFmax[values->idx];
	float peak = values->LApeak[values->idx];
	
	//printf("%d - %d\n",values->idx,values->num_elem);
	for (int i = values->idx; i < values->num_elem; i++) {
		lae = lae + values->LAE[i];		// somatorio
		if (min > values->LAFmin[i]) {
			min = values->LAFmin[i];	// minimo dos minimos
		}
		if (max < values->LAFmax[i]) {
			max = values->LAFmax[i];	// maximo dos maximos
		}
		if (peak<values->LApeak[i]) {
			peak = values->LApeak[i];	// maximos dos peaks
		}
	}
	lae = lae / (values->num_elem - values->idx);	// media do somatorio
	float laeq = lae;
	if (values->count != 0) {
		float desc = decalibrate(cal,values->LAeq[values->count - 1]);//decalibrate the calibrated value
		float lin = toLinear(desc);//passar para linear os valores em logaritmo de modo a fazer média
		laeq = laeq + lin;
		laeq = laeq/2;
	}

	//calibrar os valores
	float laeq_log = calibrate(cal, laeq);
	float lae_log = calibrate(cal, lae);
	float max_log = calibrate(cal, max);
	float min_log = calibrate(cal, min);
	float peak_log = calibrate(cal, peak);
	addFinal(values, laeq_log, max_log, min_log, lae_log, peak_log);	// adicionar	
}