#include "values.h"

//Creates the struct
Values_struct *createValue(void) {
	Values_struct *create = malloc(sizeof *create);
	create->idx = 0; //Used only for med Values
	create->count = 0;
	create->num_elem = 0;
	create->LApeak = NULL;
	create->LAFmax = NULL;
	create->LAFmin = NULL;
	create->LAE = NULL;
	create->LAeq = NULL;
	create->LApeak_buff = NULL;
	create->LAFmax_buff = NULL;
	create->LAFmin_buff = NULL;
	create->LAE_buff = NULL;
	return create;
}

void addPeak(Values_struct *values, float peak){
	values->LApeak = realloc( values->LApeak, (values->num_elem + 1) * sizeof *values->LApeak );
	values->LApeak[values->num_elem] = peak;	// add peak
}

void valueAdd(Values_struct *values, float max, float min, float lae) {
	values->LAFmax = realloc(values->LAFmax, (values->num_elem + 1) * sizeof *values->LAFmax);
	values->LAFmax[values->num_elem] = max;
	
	values->LAFmin = realloc(values->LAFmin, (values->num_elem + 1 ) * sizeof *values->LAFmin);
	values->LAFmin[values->num_elem] = min;
	
	values->LAE = realloc(values->LAE, ( values->num_elem + 1 ) * sizeof *values->LAE);
	values->LAE[values->num_elem] = lae;

	values->num_elem = values->num_elem + 1;
}

void addFinal(Values_struct *values, float laeq, float max, float min, float lae,float peak) {
	values->LAeq = realloc(values->LAeq, (values->count + 1) * sizeof *values->LAeq);
	values->LAeq[values->count] = laeq;
	
	values->LAFmax_buff = realloc(values->LAFmax_buff, (values->count + 1) * sizeof *values->LAFmax_buff);
	values->LAFmax_buff[values->count] = max;
	
	values->LAFmin_buff = realloc(values->LAFmin_buff, (values->count + 1) * sizeof *values->LAFmin_buff);
	values ->LAFmin_buff[values->count] = min;
	
	values->LAE_buff = realloc(values->LAE_buff, (values->count + 1) * sizeof *values->LAE_buff);
	values->LAE_buff[values->count] = lae;
	
	values->LApeak_buff = realloc( values->LApeak_buff, (values->count + 1) * sizeof *values->LApeak_buff);
	values->LApeak_buff[values->count] = peak;

	values->count = values->count + 1;
}
