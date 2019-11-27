// LIBS load

#include <stdlib.h>
#include <string.h>
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
#include "rtes_rpisc_server.h"
#include "rtes_rpisc_ioworker.h"


// CONSTANTS
const int   S_PORT    =   2288;
const char  S_IP[16]  =   "127.0.0.1";

// *** PROGRAM START *** //

int main(int argc, char **argv) {
	int io_thread_status;
	pthread_t io_worker_thread;
	void *thread_result;
	evthread_use_pthreads();
	io_thread_status = pthread_create (&io_worker_thread, NULL, io_worker_main, NULL);
	//if (io_thread_status != 0) err_abort (io_thread_status, "Create IO thread");
	// debug thread ID: printf("IO Thread ID: %d\n", io_worker_thread);
	server_main(S_PORT, S_IP);
	pthread_join(io_worker_thread, &thread_result);
	printf("Thread join! Faillll!\n");
	return 0;
}