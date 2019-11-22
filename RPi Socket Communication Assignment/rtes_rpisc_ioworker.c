// LIBS load
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <pthread.h>

// LIBEVENT load
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>

// FILES load
#include "rtes_rpisc_ioworker.h"

#define EVLOOP_NO_EXIT_ON_EMPTY 0x04

struct event_base *io_base;

void * io_worker_main(void *arg) {
	printf("entered thread area\n");
	// check if it runs endlessly
	io_base = event_base_new();
	event_base_loop(io_base, EVLOOP_NO_EXIT_ON_EMPTY);
	pthread_exit(0);
}

void io_handle(evutil_socket_t fd, short what, void *arg){
	printf("Handler function triggered!");
}