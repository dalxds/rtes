#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <math.h>

void alarm_handler (int sig);

struct timeval tv;
long int sig_time;

int main (int argc, char *argv[]){
	int t = 10;
	float dt = 0.1;
	
	int intervals = (int) t/dt;
	long int timestamps[intervals];

	gettimeofday(&tv, NULL);

	long int init_time = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000; 

	printf("Initial Time: %ld\n", init_time);

	signal(SIGALRM, alarm_handler); 

	for (int i = 0; i < intervals; i++) {
		timestamps[i] = sig_time - init_time;
		ualarm(dt * 1000000, 0);
		pause();
	}

	for (int i = 0; i < intervals; i++){
		printf("%ld\n", timestamps[i]);
	}
}

void alarm_handler (int sig) {
	gettimeofday(&tv, NULL);
	sig_time = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}
