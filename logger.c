
#include "common.h"


void flog(FILE* file, int trignum, SAMPLE* arr, int size)
{
	int i;
	fprintf(file, "#%d:%.2fs,", trignum, 
		(double)(trignum * SAMPLE_PERIOD_MS) / 1000);
	for(i = 0; i < size - 1; i++) {
		fprintf(file, "%d,", arr[i]);
	}
	fprintf(file, "%d\n", arr[size - 1]);
}

void flog2(FILE* file, int trignum, SAMPLE* lhs, int lhs_sz, 
	SAMPLE* rhs, int rhs_sz)
{
	int i;
	fprintf(file, "#%d:%.2fs,", trignum, 
		(double)(trignum * SAMPLE_PERIOD_MS) / 1000);
	for(i = 0; i < lhs_sz; i++) {
		fprintf(file, "%d,", lhs[i]);
	}
	for(i = 0; i < rhs_sz - 1; i++) {
		fprintf(file, "%d,", rhs[i]);
	}
	fprintf(file, "%d\n", rhs[rhs_sz - 1]);
}