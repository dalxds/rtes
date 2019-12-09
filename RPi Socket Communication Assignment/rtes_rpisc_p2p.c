// LIBS load
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

// LIBEVENT load
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

// LIBEVENT threads
#include "event2/event-config.h"
#include "event2/thread.h"

// FILES load
#include "rtes_rpisc_p2p.h"
#include "rtes_rpisc_server.h"
#include "rtes_rpisc_ioworker.h"
#include "rtes_rpisc_rwlock.h"

// GLOBAL VARIABLES & CONSTANTS
const int   S_PORT    				=   2288;
const char  S_IP[INET_ADDRSTRLEN]  	=   "127.0.0.1";

// STRUCTS

// TODO: align structs in desceding order

// GLOBAL STRUCT ARRAYS
pthread_t threads_pool[THREADS_NUM];

// *** PROGRAM START *** //

// PARSER
void nodes_list_parser() {
	char fname[] = "nodes_list.txt";
	FILE *fptr = NULL;
	int i = 0;
	fptr = fopen(fname, "r");

	for (i = 0; i < NODES_NUM; i++) {
		nodes_list -> id = i;
		if(fgets(&nodes_list[i].ip[0], INET_ADDRSTRLEN, fptr)) {
			strtok(&nodes_list[i].ip[0], "\n");
		}
	}
	/*
	 * Debugging
	 */
	// for (i = 0; i < NODES_NUM; i++) {
	// 	if(strcmp(&nodes_list[i].ip[0], "\0")) 
	// 		printf(" %s\n", &nodes_list[i].ip[0]);
	// }
}

//MAIN

int main(int argc, char **argv) {
	// initiliaze variables
	int status;
	// run parser
	nodes_list_parser();
	// use pthreads in Libevent base
	evthread_use_pthreads();
	/*** IO Thread ***/
	status = pthread_create (&threads_pool[0], NULL, io_worker_main, NULL);
	if (status != 0) err_abort (status, "Create thread");

	////// IO Thread debbugging
	//if (io_thread_status != 0) err_abort (io_thread_status, "Create IO thread");
	// debug thread ID: printf("IO Thread ID: %d\n", io_worker_thread);
	//pthread_join(io_worker_thread, &thread_result);
	//printf("Thread join! Faillll!\n");

	/*** Data Worker ***/
	/// write here
	/*** Server Thread ***/
	server_main(S_PORT, S_IP);
	/*** Client Thread ***/
	/// write here
	// exit main
	return 0;
}