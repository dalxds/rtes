#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <math.h>

int main (int argc, char *argv[]){
	if (argc < 2) {
		printf("No proper arguments given!\nExiting...\n");
		exit(1);
	}

	int t = atoi(argv[1]);
	float dt = atof(argv[2]);

	long diff = 0;
	int intervals = round(t/dt) + 1;

	long int timestamps[intervals];
	long int samples[intervals-2];
	long int ref[intervals];

	//create REFERENCE table
	ref[0] = 0;
	for (int i = 1; i < intervals; i++){
		ref[i] = ref[i-1] + dt * 1000;
	}

	//get INITIAL TIME
	struct timeval tv;
	gettimeofday(&tv, NULL);

	long int init_time = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000; 

	printf("Initial Time: %ld\n", init_time);

	for (int i = 0; i < intervals; i++) {
		//get timestamp
		gettimeofday(&tv, NULL);
		long int sig_time = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

		//write timestamp to table
		timestamps[i] = sig_time - init_time;
		//calculcate difference
		diff = timestamps[i] - ref[i];

		// PRINT DATA IN EACH ITERATION
		// printf("TimeStamp: %ld\n", timestamps[i]);
		// printf("Reference: %ld\n", ref[i] );
		// printf("Diff: %ld\n", diff );

		//call sleep with calculated difference
		usleep((dt*1000 - diff) * 1000);
	}

	//create SAMPLES table
	for (int i = 2; i <= intervals; i++) {
		samples[i-2] = timestamps[i] - timestamps[i-1];
	}

	/* WRITE DATA TO FILES */

	//FILE #1 - TIMESTAMPS

	FILE *fp1 = fopen("/Users/Dimitris/Desktop/enhancedSleep_timestamps.txt","w");

	if (fp1 == NULL){
		printf("Error With File!");   
		exit(1);
   	}

    	for (int i = 1; i < intervals; i++){
		fprintf(fp1, "%ld\n", timestamps[i]);
	}

	fclose(fp1);

	//FILE #2 - SAMPLES (DIFFERENCES BETWEEN TIMESTAMPS)

	FILE *fp2 = fopen("/Users/Dimitris/Desktop/enhancedSleep_samples.txt","w");

	if (fp2 == NULL){
		printf("Error With File!");   
		exit(1);
   	}

	for (int i = 0; i < intervals-2; i++){
		fprintf(fp1, "%ld\n", samples[i]);
	}

	fclose(fp2);

	//FILE #3 - REFERENCE

	FILE *fp3 = fopen("/Users/Dimitris/Desktop/enhancedSleep_reference.txt","w");

	if (fp3 == NULL){
		printf("Error With File!");   
		exit(1);
   	}

    	for (int i = 1; i < intervals; i++){
		fprintf(fp3, "%ld\n", ref[i]);
	}

	fclose(fp3);
}
