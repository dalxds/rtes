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
#include <event2/thread.h>

// FILES load
#include "rtes_rpisc_ioworker.h"
#include "rtes_rpisc_dataworker.h"
#include "rtes_rpisc_nodeslist.h"
#include "rtes_rpisc_p2p.h"

// GLOBAL VARIABLES & CONSTANTS
#define EVLOOP_NO_EXIT_ON_EMPTY 0x04

// STRUCTS
struct event_base *io_base;

// *** PROGRAM START *** //

//TODO: Handle Connection Closing

void *io_worker_main(void *arg) {
    printf("[IO] Entered Thread Area\n");
    io_base = event_base_new();
    // signal other threads
    IO_BASE_STARTED = true;
    //NOTE: use EVLOOP_NON_BLOCK or not?
    event_base_loop(io_base, EVLOOP_NO_EXIT_ON_EMPTY);
    printf("pthread will exit...fireeee!\n");
    pthread_exit(0);
}

void io_handle_read(struct bufferevent *bev, void *arg) {
    printf("[IO] Read Callback Triggered!\n");

    if (bufferevent_read_buffer(bev, dw_buffer) < 0)
        printf("Error on reading buffer\n");
}

void io_handle_events(struct bufferevent *bev, short events, void *user_data) {
    if (events & BEV_EVENT_EOF)
        printf("Connection closed.\n");

    else if (events & BEV_EVENT_ERROR) {
        printf("Got an error on the connection: %s\n",
               strerror(errno));/*XXX win32*/
    }

    /* None of the other events can happen here, since we haven't enabled
     * timeouts */
    bufferevent_free(bev);
}