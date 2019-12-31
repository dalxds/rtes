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
#include <event2/event.h>
#include "event2/event-config.h"
#include "event2/thread.h"

// FILES load
#include "rtes_rpisc_p2p.h"
#include "rtes_rpisc_server.h"
#include "rtes_rpisc_ioworker.h"
#include "rtes_rpisc_dataworker.h"
#include "rtes_rpisc_rwlock.h"
#include "rtes_rpisc_nodeslist.h"


// // GLOBAL VARIABLES & CONSTANTS
const uintptr_t S_PORT = 2288;
volatile bool IO_BASE_STARTED = false;
volatile bool NODES_LIST_INIT_DONE = false;

// STRUCTS

/*  THREADS POOL
 *      0: io worker
 *      1: data worker
 *      2: server
 *      3: client
 */
pthread_t threads_pool[THREADS_NUM];

//* NOTES
/// 1. No cleanup because it should work non stop
/// 2. No memory management
/// 3. [DW] if many nodes a new array with connected nodes would've been created

// *** PROGRAM START *** //
int main(int argc, char **argv) {
    // initiliaze variables
    int status;
    // use pthreads in Libevent base
    evthread_use_pthreads();
    /*** IO Thread ***/
    status = pthread_create (&threads_pool[0], NULL, io_worker_main, NULL);
    if (status != 0) 
        err_abort (status, "[IO] Create thread");
    printf("IO Thread ID: %d\n", (int)threads_pool[0]);

    ////// IO Thread debbugging
    //if (io_thread_status != 0) err_abort (io_thread_status, "Create IO thread");
    // debug thread ID: printf("IO Thread ID: %d\n", io_worker_thread);
    //pthread_join(io_worker_thread, &thread_result);
    //printf("Thread join! Faillll!\n");

    while(!IO_BASE_STARTED)
        sleep(0);

    // run parser
    nodes_list_init();

    while(!NODES_LIST_INIT_DONE)
        sleep(0);

    /*** Data Worker ***/
    status = pthread_create (&threads_pool[1], NULL, data_worker_main, NULL);
    if (status != 0) 
        err_abort (status, "[DW] Create thread");
    printf("Data Worker ID: %d\n", (int)threads_pool[1]);
    /*** Server Thread ***/
    // status = pthread_create (&threads_pool[2], NULL, server_main, (void*)S_PORT);
    // if (status != 0) 
    //     err_abort (status, "[SE] Create thread");
    // printf("Server Thread ID: %d\n", (int)threads_pool[2]);
    /*** Client Thread ***/
    status = pthread_create (&threads_pool[3], NULL, client_main, NULL);
    if (status != 0) 
        err_abort (status, "[CL] Create thread");
    printf("Client Thread ID: %d\n", (int)threads_pool[3]);
    /*** Client Thread ***/
    status = pthread_join (threads_pool[0], NULL);
        if (status != 0)
            err_abort (status, "Join thread");
    // exit main
    return 0;
}