#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <math.h>


int main (int argc, char *argv[]){
	int t = 10;
	float dt = 0.1;
	long diff = 0;
	
	int intervals = (int) t/dt;
	long int timestamps[intervals+1];

	int ref[intervals+1];

	for (int i = 0; i < intervals+1; i++){
		ref[i] = i * 100;
	}

	// for (int i = 0; i <= intervals; i++){
	// 	printf("%d\n", ref[i]);
	// }

	struct timeval tv;
	gettimeofday(&tv, NULL);

	long int init_time = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000; 

	printf("Initial Time: %ld\n", init_time);

	for (int i = 0; i < intervals+1; i++) {
		gettimeofday(&tv, NULL);
		long int sig_time = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

		timestamps[i] = sig_time - init_time;
		printf("TimeStamp: %ld\n", timestamps[i]);
		printf("Reference: %d\n", ref[i] );

		diff = timestamps[i] - ref[i];
		printf("Diff: %ld\n", diff );

		usleep((dt*1000 - diff) * 1000);
	}

	printf("============\nPrinting Table\n============ \n");

	for (int i = 0; i < intervals+1; i++){
		printf("%ld\n", timestamps[i]);
	}
}
