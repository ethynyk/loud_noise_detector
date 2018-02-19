#include <stdlib.h>

#include <pthread.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "common.h"

static void usage(int argc, char** argv)
{
	printf("%s <source file>\n", argv[0]);
	printf("\tsource file: file name to parse for loud noises.\n");
	printf("\t\tIf no source file provided, default samples.bin.\n");
}


int print_errors(const char* msg, int err_num)
{
	printf("%s (%d): %s\n", msg, err_num, strerror(err_num));
	return -1;
}


int main(int argc, char** argv)
{
	int ret;
	pthread_t read_t, process_t;
	struct synchro_data thread_data;

	if(argc > 2) {
		usage(argc, argv);
		return -1;
	}

	if(pipe(thread_data.pipefd) == -1) {
		return print_errors("Failed to open pipes", errno);
	}

	if(argc == 2) {
		thread_data.audiofd = open(argv[1], O_RDONLY);
	}
	else {
		thread_data.audiofd = open(AUDIOFILENAME, O_RDONLY);
	}
	
	if(thread_data.audiofd < 0) {
		usage(argc, argv);
		return print_errors("Error opening source file", errno);
	}

	thread_data.logfile = fopen(LOGFILENAME, "w+");
	if(thread_data.logfile == NULL) {
		return print_errors("Failed to open log file", errno);
	}

	ret = pthread_create(&read_t, NULL, read_samples, (void*)&thread_data);
	if(ret != 0) {
		return print_errors("Error starting read thread", ret);
	}

	ret = pthread_create(&process_t, NULL, process_samples, 
		(void*)&thread_data);
	if(ret != 0) {
		return print_errors("Error starting process thread", ret);
	}

	pthread_join(read_t, NULL);
	pthread_join(process_t, NULL);

	printf("Done\n");
	fclose(thread_data.logfile);
	close(thread_data.audiofd);
	close(thread_data.pipefd[0]);
	close(thread_data.pipefd[1]);
}