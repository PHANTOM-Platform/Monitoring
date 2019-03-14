#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <gmp.h>
#include <time.h> 
#include "pi_code.h"

// gcc  -std=gnu99 -O2 -march=native -fomit-frame-pointer -o pi_simple pi_code.c pi_simple.c -lm -lgmp -lpthread
// ./pi 2 8000 2000
long long int mycurrenttime (void) {
	struct timespec ts_start;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts_start);
	long long int timeus = (long int) (ts_start.tv_sec*1000000000LL +  ts_start.tv_nsec);
    return timeus;
}

void threadedChudnovsky(unsigned long threadCount, mpf_t rop, unsigned long precision, unsigned long itercount) {
	memset( &workInfo, 0, sizeof(workInfo_t) );
	// Initialization and creation of workers
	workInfo.precision = precision;
	workInfo.workers = malloc(sizeof(worker_t) * threadCount);
	memset(workInfo.workers, 0, sizeof(worker_t) * threadCount);
	for (unsigned long i=0; i<(threadCount-1); i++) {
		mpf_init2( workInfo.workers[i].rop, precision );
		workInfo.workers[i].iterStart = roundDiv(itercount, threadCount) * i;
		workInfo.workers[i].iterFinish = roundDiv(itercount, threadCount) * (i+1);
		unsigned long *workerId = malloc(sizeof(unsigned long));
		*workerId = i;
		pthread_create( &(workInfo.workers[i].thread), NULL, doChudnovsky, workerId );
	}
	mpf_init2( workInfo.workers[threadCount-1].rop, precision );
	workInfo.workers[threadCount-1].iterStart = roundDiv(itercount, threadCount) * (threadCount-1);
	workInfo.workers[threadCount-1].iterFinish = itercount;
	unsigned long *workerId = malloc(sizeof(unsigned long));
	*workerId = threadCount-1;
	pthread_create( &(workInfo.workers[threadCount-1].thread), NULL, doChudnovsky, workerId );
	// esperar que cada worker acabe
	for (unsigned long i=0; i<threadCount; i++) {
		pthread_join( workInfo.workers[i].thread, NULL );
		mpf_add(rop, rop, workInfo.workers[i].rop);
	}
	// temos (1/PI); inverter
	mpf_ui_div(rop, 1, rop);
}

int main(int argc, char *argv[]) {
	unsigned long threadCount, itercount, precision;
	long long int start_time=mycurrenttime();
	if (argc<4) {
		fprintf(stderr, "Missing arguments..\n");
		fprintf(stderr, "\t%s  Nr_threads  Nr_iterations  Precision_in_bytes\n", argv[0]);
		return -1;
	}
	threadCount = atoi(argv[1]);
	itercount = atoi(argv[2]);
	precision = 8*atoi(argv[3]);

	if (itercount < threadCount) {
		fprintf(stderr, "Error, We can not have more threads than iterations\n");
		return -1;
	}

	mpf_t pi;
	mpf_init2(pi, precision);
	threadedChudnovsky(threadCount, pi, precision, itercount);
	char *pi_str;
	gmp_asprintf( &pi_str, "%.Ff", pi );
	unsigned significantDigits = strlen(pi_str) - 2;
	printf("Nr decimal places: %u\n%s\n", significantDigits, pi_str);
	// ***** finishing_program ****
	long long int end_time=mycurrenttime();
	printf(" TOTAL EXECUTION TIME:%9lli s ", (end_time - start_time)/1000000000LL);
		long long int temp_time = (end_time - start_time)%1000000000LL;
	printf(" +  %3lli ms ", temp_time/1000000);
		temp_time = (end_time - start_time)%1000000LL;
	printf(" +  %3lli us ", temp_time/1000);
		temp_time = (end_time - start_time)%1000LL;
	printf(" +  %3lli ns \n", temp_time);
	return 0;
}
