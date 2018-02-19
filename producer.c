#include <time.h>

#include <sys/select.h>
#include <sys/timerfd.h>

#include "common.h"

static int create_periodic(int* fd, int period)
{
	int sec, nsec, ret;
	struct itimerspec itval;

	*fd = timerfd_create(CLOCK_MONOTONIC, 0);
	if(*fd < 0) {
		return *fd;
	}

	sec = period / 1000;
	nsec = (period - (sec * 1000)) * 1000000;

	itval.it_interval.tv_sec = sec;
	itval.it_interval.tv_nsec = nsec;

	itval.it_value.tv_sec = sec;
	itval.it_value.tv_nsec = nsec;

	ret = timerfd_settime(*fd, 0, &itval, NULL);
	return ret;
}

static int clock_select(int fd)
{
	int ret;
	fd_set readfds;

	do {
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		ret = select(fd + 1, &readfds, NULL, NULL, NULL);
	}
	while(ret == -1 && errno == EINTR);

	if(ret > 0) {
		if(FD_ISSET(fd, &readfds)) {
			unsigned long long expires;
			ret = read(fd, &expires, sizeof(expires));
			if(ret < 0) {
				return errno;
			}
			return expires;
		}
	}
	return errno;
}

void* read_samples(void* arg)
{
	int clockfd, ret;
	ssize_t bytes_read;
	struct synchro_data* sync = (struct synchro_data*)arg;
	SAMPLE buf[PROCESS_ACCUMULATE];
	int buf_cnt = 0;

	ret = create_periodic(&clockfd, SAMPLE_PERIOD_MS);
	if(ret < 0) {
		print_errors("Could not start periodic clock", errno);
		return NULL;
	}

	do {
		ret = clock_select(clockfd);
		if(ret != 1) {
			// TODO: handle this error condition
			// either no expires happened but the clock woke up
			// or too many expires happened
			// or an error occured
			printf("Expires since last read: %d\n", ret);
		}

		bytes_read = read(sync->audiofd, &buf[buf_cnt], SAMPLE_SIZE);

		if(bytes_read < 0) {			// error
			print_errors("Cannot read bytes from file", bytes_read);
			return NULL;
		}
		else if(bytes_read < SAMPLE_SIZE) { // bytes read and EOF
			if(buf_cnt != (PROCESS_ACCUMULATE - 1)) {
				memset(&buf[buf_cnt + 1], 0, 
				(PROCESS_ACCUMULATE - buf_cnt - 1) * SAMPLE_SIZE);	// zeros at end
				buf_cnt = PROCESS_ACCUMULATE - 1;
			}
			bytes_read = 0;
		}
		buf_cnt++;
		if(buf_cnt == PROCESS_ACCUMULATE) {
			buf_cnt = 0;
			write(sync->pipefd[PIPEWRITE], buf, sizeof(buf));
		}
	}
	while(bytes_read > 0);

	write(sync->pipefd[PIPEWRITE], &ret, sizeof(ret));	// this kills consumer
		
	close(clockfd);

	return NULL;
}