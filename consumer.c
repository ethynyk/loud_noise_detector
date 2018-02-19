#include <math.h>
#include <stdlib.h>

#include <sys/select.h>

#include "common.h"


#define WINDOW_SIZE		(PROCESS_ACCUMULATE / 2)


static double calc_dist(double avg, SAMPLE pt)
{
	return fabs(pt - avg);
}

static double updiv(int sum, int div)
{
	return (((double)sum) / div);
}

static void peak_detect(SAMPLE* lhs, SAMPLE* rhs, int start, FILE* file)
{
	static int above = 0;
	int i;
	double mavg;
	int rsum = 0;
	int div = 0;

	for(i = 0; i < WINDOW_SIZE; i++) {
		rsum += abs(lhs[i]);
		div++;
	}

	mavg = updiv(rsum, div);

	for(i = WINDOW_SIZE; i < PROCESS_ACCUMULATE; i++) {
		if(calc_dist(mavg, abs(lhs[i])) > THRESHOLD) {
			if(above == 0) {
				above = 1;
				if(i == WINDOW_SIZE) {
					flog(file, start + i, lhs, PROCESS_ACCUMULATE);
				}
				else {
					int lhs_i = i - WINDOW_SIZE;
					int lhs_sz = PROCESS_ACCUMULATE - lhs_i;
					int rhs_i = 0;
					int rhs_sz = PROCESS_ACCUMULATE - lhs_sz;
					flog2(file, start + i, &lhs[lhs_i], lhs_sz,
							&rhs[rhs_i], rhs_sz);
				}
			}
		}
		else {
			above = 0;
			rsum += abs(lhs[i]);
			div++;
			mavg = updiv(rsum, div);
		}
	}
	for(i = 0; i < PROCESS_ACCUMULATE - WINDOW_SIZE; i++) {
		if(calc_dist(mavg, abs(rhs[i])) > THRESHOLD) {
			if(above == 0) {
				above = 1;
				if(i == WINDOW_SIZE) {
					flog(file, start + i + PROCESS_ACCUMULATE, 
						rhs, PROCESS_ACCUMULATE);
				}
				else {
					int rhs_i = 0;
					int rhs_sz = i + WINDOW_SIZE + 1;
					int lhs_i = PROCESS_ACCUMULATE + i - WINDOW_SIZE;
					int lhs_sz = abs(i - WINDOW_SIZE);
					
					flog2(file, start + i, &lhs[lhs_i], lhs_sz,
							&rhs[rhs_i], rhs_sz);
				}
			}
		}
		else {
			above = 0;
			rsum += abs(rhs[i]);
			div++;
			mavg = updiv(rsum, div);
		}
	}

}

static void swap_ptr(SAMPLE** lhs, SAMPLE** rhs)
{
	SAMPLE* temp;
	temp = *lhs;
	*lhs = *rhs;
	*rhs = temp;
}

void* process_samples(void* arg)
{
	int ret, bytes_read;
	uint64_t total_samples = 0;
	fd_set pfds;
	SAMPLE* old_buf;
	SAMPLE* new_buf;
	SAMPLE buf[2][PROCESS_ACCUMULATE];
	struct synchro_data* sync = (struct synchro_data*)arg;

	memset(buf, 0, sizeof(buf));
	old_buf = &buf[0][0];
	new_buf = &buf[1][0];

	do {
		do {
			FD_ZERO(&pfds);
			FD_SET(sync->pipefd[PIPEREAD], &pfds);
			ret = select(sync->pipefd[PIPEREAD] + 1, &pfds, NULL, NULL, NULL);
		}
		while(ret == -1 && errno == EINTR);

		if(ret > 0) {
			if(FD_ISSET(sync->pipefd[PIPEREAD], &pfds)) {
				bytes_read = read(sync->pipefd[PIPEREAD], new_buf, 
					PROCESS_ACCUMULATE * SAMPLE_SIZE);
				if(bytes_read < 0) {
					print_errors("Could not read from pipe", errno);
				}
				if(bytes_read == PROCESS_ACCUMULATE * SAMPLE_SIZE) {
					peak_detect(old_buf, new_buf, total_samples, sync->logfile);
					swap_ptr(&old_buf, &new_buf);
					total_samples += PROCESS_ACCUMULATE;
				}
			}
		}
	}
	while(bytes_read == PROCESS_ACCUMULATE * SAMPLE_SIZE);
	return NULL;
}