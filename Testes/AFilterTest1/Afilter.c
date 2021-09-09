#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <math.h>


//----Input---   
    //u = {1,2,3,4,5}
    //size = 5
   // x = 50
//---Output---
  //u = {50,1,2,3,4}
  
void shift_right(float* u, int size,float x) {
    
   memmove(&u[1],&u[0],size *sizeof(float));
   u[0]=x;

}

void AFilter(float *x,float *y, int size, int N, float*a,float*b,float *u){
	int u_size = N;//Tamanho da variável de estado
	
	for(int n = 0; n<size;n++){
		
		shift_right(u,u_size,x[n]);//shifta para o lado e u[n] = x[n] => u[0] = x[n]
		
		for(int i = 1;i<=N;i++){
			u[0] = u[0] + (((-1)*a[i])*u[i]);//u(n) = u(n) + (-1)a(i)*u(i)
		}
		for(int i = 0; i<=N;i++){
			y[n] = y[n] + (b[i]*u[i]);//y(n) = y(n)+b(i)*u(n-i)
			
		}
		
		
	}
		
}


int main(){
	
	int N = 6;//Filter order
	
	//Input signal
	
	float x[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int size = sizeof(x)/sizeof(*x);
	
	//Coefficients
	float b[] = {0.169994948147430,0.280415310498794,-1.120574766348363,0.131562559965936,0.974153561246036,-0.282740857326553,-0.152810756202003};
	float a[] = {1.00000000000000000,-2.12979364760736134,0.42996125885751674,1.62132698199721426,-0.96669962900852902,0.00121015844426781,0.04400300696788968};
	
	//float a[] = {1,-0.5095};
	//float b[] = {0.2452,0.2452};
	
	float *y = calloc(size, sizeof(float));//y=0
	float *u = calloc(N+1, sizeof(float));
	
	
	printf("x = ");
	for(int n = 0; n<size;n++){
		printf(" %lf ",x[n]);
		
	}	
	AFilter(x,y,size,N,a,b,u);
	
	
	memcpy(x,y,size*sizeof(float));
	
	printf("\n");
	printf("y = ");
	for(int n = 0; n<size;n++){
		printf(" %lf ",x[n]);
		
	}	
	printf("\n");

}
