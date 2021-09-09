#include "filter.h"



//Inits time weight filter
TimeWeight_struct *initTimeWeightFILTER(){
	TimeWeight_struct *tw=malloc(sizeof*tw);
	tw->previous = 0;
	return tw;
}


void timeWeightFILTERING(Buffer *buff,int size,TimeWeight_struct *tw){
	//y[n]=(1−α)x[n]+αy[n−1]
	for(int i =0; i<size; i++){
		
		buff->buff[i] = ((1-a) * buff->buff[i]) + (a * tw->previous);
		tw->previous = buff->buff[i];
		
	}
	
}

/*
 u = [1,2,3,4,5]
 shiftright(u,5,50)
 u=[50,1,2,3,4]
 */

static void shift_right(float* u, int size,float x) {//insere o valor da posição 0 e shifta tudo pra direita
    
   memmove(&u[1],&u[0],size *sizeof(float));
   u[0]=x;

}

Afilter_struct *initAFILTER(float *coef_a,float *coef_b,int N){
	Afilter_struct *af=malloc(sizeof*af);
	af->coef_a = coef_a;
	af->coef_b = coef_b;
	af->u=calloc(N+1,sizeof(float));
	af->N = N;
	return af;
}


void AfilterFILTERING(Buffer *buff,int size,Afilter_struct *af){
	float *y = calloc(size,sizeof(float));
		
	for(int n = 0; n<size;n++){
		
		shift_right(af->u,af->N,buff->buff[n]/(float)NORM);//shifta para o lado e u[n] = x[n] => u[0] = x[n]
		
		for(int i = 1;i<=af->N;i++){
			af->u[0] = af->u[0] + (((-1)*af->coef_a[i])*af->u[i]);//u(n) = u(n) + (-1)a(i)*u(n-i)
		}
		for(int i = 0; i<=af->N;i++){
			y[n] = y[n] + (af->coef_b[i]*af->u[i]);//y(n) = y(n)+b(i)*u(n-i)		
		}
		
	}
	memcpy(buff->buff,y,size*sizeof(float));
	free(y);
	
	
}
