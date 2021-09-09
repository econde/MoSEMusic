#ifndef _VALUES_H_
#define _VALUES_H_

#include <stdlib.h>

typedef struct{
	int idx;//sets the start of one new second on the arrays
	int count;//number of 1 second values
	int num_elem;//number of elements in linear values array
	float *LApeak;//linear
	float *LAFmax;//linear
	float *LAFmin;//linear
	float *LAE;//linear
//----------------values meaasured each second
	float *LAeq;//log
	float *LApeak_buff;//log
	float *LAFmax_buff;//log
	float *LAFmin_buff;//log
	float *LAE_buff;//log
	
}Values_struct;


Values_struct *createValue(void);//Creates the struct
void addPeak(Values_struct *values, float peak);//add peak to linear peak array
void valueAdd(Values_struct *values,float max, float min, float lae);//add lmax, lmin e lae aos arrays lieares correspondentes
void addFinal(Values_struct *values, float laeq,float max, float min, float lae,float peak);//adição final

#endif
