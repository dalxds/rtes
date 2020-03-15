// LIBRARIES
#include <signal.h>
#include <string.h>
#include <pthread.h>

/// LIBEVENT
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event.h>
#include "event2/event-config.h"
#include "event2/thread.h"

// FILES
#include "rtes_rpisc_p2p.h"
#include "rtes_rpisc_server.h"
#include "rtes_rpisc_client.h"
#include "rtes_rpisc_ioworker.h"
#include "rtes_rpisc_dataworker.h"
#include "rtes_rpisc_rwlock.h"
#include "rtes_rpisc_nodeslist.h"

// GLOBAL VARIABLES & CONSTANTS
const uint32_t  THIS_AEM    = 8607;
const uintptr_t PORT        = 2288;

// THREADS' INIT CONTROL
volatile    bool    IO_BASE_STARTED         = false;
volatile    bool    NODES_LIST_INIT_DONE    = false;
volatile    bool    DATA_WORKER_INIT        = false;

// STRUCTS
/*  THREADS POOL
 *      0: io worker
 *      1: data worker
 *      2: server
 *      3: client
 */
pthread_t threads_pool[THREADS_NUM];

/********************************************//**
 *  PROGRAM START
 ***********************************************/
int main(int argc, char **argv) {
    // initiliaze variables
    int status;
    // use pthreads in Libevent base
    evthread_use_pthreads();
    // SIGPIPE hanlder
    signal(SIGPIPE, SIG_IGN);

    /*** IO Thread ***/
    status = pthread_create (&threads_pool[0], NULL, io_worker_main, NULL);
    if (status != 0) 
        err_abort(status, "[IO] Create thread");
    printf("[PT] IO Thread ID: %d\n", (int)threads_pool[0]);

    /// black hole for IO Base to initialize
    while(!IO_BASE_STARTED);

    // run parser
    nodes_list_init();

    /// black hole for nodes_list initialization to finish
    while(!NODES_LIST_INIT_DONE);

    /*** Data Worker ***/
    status = pthread_create (&threads_pool[1], NULL, data_worker_main, NULL);
    if (status != 0) 
        err_abort (status, "[DW] Create thread");
    printf("[PT] Data Worker ID: %d\n", (int)threads_pool[1]);

    /// black hole for data worker to initialize
    while(!DATA_WORKER_INIT);

    /*** Server Thread ***/
    status = pthread_create (&threads_pool[2], NULL, server_main, (void*)PORT);
    if (status != 0) 
        err_abort (status, "[SE] Create thread");
    printf("[PT] Server Thread ID: %d\n", (int)threads_pool[2]);

    /*** Client Thread ***/
    status = pthread_create (&threads_pool[3], NULL, client_main, (void*)PORT);
    if (status != 0) 
        err_abort (status, "[CL] Create thread");
    printf("[PT] Client Thread ID: %d\n", (int)threads_pool[3]);

    /*** Join Threads ***/
    status = pthread_join (threads_pool[0], NULL);
        if (status != 0)
            err_abort (status, "Join thread");

    // exit program
    return 0;
}