#include <stdint.h>
#include <stdio.h>

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <unistd.h>


/*
 *	Filename to read "audio" data from
 */
#define AUDIOFILENAME			"samples.bin"

#define LOGFILENAME				"log.csv"


/*
 * Period to read SAMPLE_SIZE bytes from FILENAME in ms
 */
#define SAMPLE_PERIOD_MS	50		//ms

typedef int16_t SAMPLE;


/*
 * Number of bytes to read from FILENAME every SAMPLE_PERIOD_MS ms
 */
#define SAMPLE_SIZE			(sizeof(SAMPLE))

#define THRESHOLD_PERC		(0.35)

#define THRESHOLD			(THRESHOLD_PERC * pow(0x80, SAMPLE_SIZE))		// bytes


/*
 * Number of samples to accumulate in process function
 */
#define PROCESS_ACCUMULATE	11


#define	PIPEREAD  	0
#define	PIPEWRITE  	1


struct synchro_data {
	int pipefd[2];
	int audiofd;
	FILE* logfile;
};

int print_errors(const char* msg, int err_num);

void* read_samples(void* arg);

void* process_samples(void* arg);

void flog(FILE* file, int trignum, SAMPLE* arr, int size);

void flog2(FILE* file, int trignum, SAMPLE* lhs, int lhs_sz, 
	SAMPLE* rhs, int rhs_sz);