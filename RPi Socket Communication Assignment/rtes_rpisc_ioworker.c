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

// GLOBAL VARIABLES & CONSTANTS
#define EVLOOP_NO_EXIT_ON_EMPTY 0x04
char msg[1000] = "";

// STRUCTS
struct event_base *io_base;

// *** PROGRAM START *** //

void *io_worker_main(void *arg) {
	printf("entered thread area\n");
	// TODO: check if it runs endlessly
	io_base = event_base_new();
	event_base_loop(io_base, EVLOOP_NO_EXIT_ON_EMPTY);
	pthread_exit(0);
}

void io_handle_read(struct bufferevent *bev, void *arg) {
	// read data
	if (bufferevent_read(bev, msg, 100) < 0)
		printf("Error on reading buffer");

	// print messages
	printf("Message Read from Client:\n");
	printf("%s\n", msg);
}

void io_handle_write(struct bufferevent *bev, void *arg) {
	char msg[] = "Hello Client (sent from server)";

	if (bufferevent_write(bev, msg, strlen(msg)) < 0)
		printf("Error on writing buffer");
}