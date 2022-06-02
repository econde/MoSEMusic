#ifndef _FILTER_H_
#define _FILTER_H_

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "buff.h"

#define NORM pow(2,(sizeof (short)*8)-1)//normalização de amostras de 16 bits
#define a 1.813894426370144*pow(10,-4)

typedef struct{
	
	float previous;//saves y[n-1]
	
}TimeWeight_struct;

typedef struct{
	
	float *u;//variável de estado
	float *coef_a;//coeficientes do filtro
	float *coef_b;//coeficientes do filtro
	int N;//Ordem do filtro
	
}Afilter_struct;


TimeWeight_struct *initTimeWeightFILTER();//Inicializar a estrutura
void timeWeightFILTERING(Buffer *buff,int size,TimeWeight_struct *tw);//filtração


Afilter_struct *initAFILTER(float *coef_a,float *coef_b,int N);//inicializar a estrutura
void AfilterFILTERING(Buffer *buff,int size,Afilter_struct *af);//filtração

#endif
