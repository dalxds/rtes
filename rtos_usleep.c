#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <math.h>


int main (int argc, char *argv[]){
	int t = 10;
	float dt = 0.1;
	
	int intervals = (int) t/dt;
	long int timestamps[intervals];

	struct timeval tv;
	gettimeofday(&tv, NULL);

	long int init_time = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000; 

	printf("Initial Time: %ld\n", init_time);

	for (int i = 0; i < intervals; i++) {
		gettimeofday(&tv, NULL);
		long int sig_time = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
		timestamps[i] = sig_time - init_time;
		usleep(dt * 1000000);
	}

	/*Print Table*/

	for (int i = 0; i < intervals; i++){
		printf("%ld\n", timestamps[i]);
	}
}
