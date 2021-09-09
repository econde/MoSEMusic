#include <math.h>
#include <stdio.h>

#define a 1.813894426370144*pow(10,-4)


int main(){
	float previous = 0;
	float x[] = {1, 1, 0, 1, 0, 0, 0, 0};
	printf(" y = ");
	for(int i = 0; i<8;i++){
		x[i] = ((1-a) * x[i]) + (a * previous);
		previous = x[i];
		printf("%lf ",x[i]);
		
	}
	
}

