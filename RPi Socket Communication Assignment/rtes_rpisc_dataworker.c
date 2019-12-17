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
#include <math.h>

// LIBEVENT load
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/thread.h>

// FILES load
#include "rtes_rpisc_p2p.h"
#include "rtes_rpisc_ioworker.h"
#include "rtes_rpisc_ringbuffer.h"
#include "rtes_rpisc_nodeslist.h"


// GLOBAL VARIABLES & CONSTANTS
#define EVLOOP_NO_EXIT_ON_EMPTY 0x04

// *** PROGRAM START *** //

void *data_worker_main(void *arg) {
    printf("[DW] Entered Thread Area.\n");

    // INIT circular buffer
    printf("[DW] Circular Buffer Initialization\n");
    msg *buffer_obj  = malloc(NODES_NUM * MSG_SIZE);
    cbuf circular_buffer = circular_buf_init(buffer_obj, NODES_NUM);

    // INIT Data Worker ev buffer
    printf("[DW] Data Worker evbuffer Initialization\n");
    extern struct evbuffer *dw_buffer;
    dw_buffer = evbuffer_new();
    if (evbuffer_enable_locking(dw_buffer, NULL) < 0)
        printf("Error on the dw_buffer lock!\n");

    // INIT data structures to be used in the endless loop
    struct msg *msg = malloc(MSG_SIZE);
    char input[MSG_SIZE];
    char output[MSG_SIZE];
    size_t loop_lim;
    int found;

    /* DEBUG_SECTION - START */

    bool printer = false;

    while (1) {
        if (!printer) {
            printf("[DW] BLACK HOLE!\n");
            printer = true;
        }
    }

    /* DEBUG_SECTION - END */

    while (1) {
        // GET DATA READY FOR OUTPUT
        for (int node_index = 0; node_index < NODES_NUM; node_index++) {
            //printf("[DW] Add to output buffer of nodes!\n");
            if (node_connected(node_index)) {
                // check if messages from circular buffer up to the latest have been written to node's buffer
                // FIXME when buffer is 0 and cbuf_index is 0 doesn't continue and creates seg fault
                if (node_cbuf_index(node_index) > circular_buf_index(circular_buffer))
                    continue;
                for (int i = 0; i < 10; i++) {
                    // read message from circular buffer with the index
                    circular_buf_read(circular_buffer, msg, node_cbuf_index(node_index));
                    // destructure data (function in ringbuffer file)
                    circular_buf_msg_destructure(msg, output);
                    // add to output buffer as string
                    node_add_to_output_buffer(node_index, output);
                    // increment node's buffer index
                    node_inc_cbuf_index(node_index);
                }
            }
        }
        // ADD DATA FROM INPUT TO CIRCULAR BUFFER
        loop_lim = fmin((evbuffer_get_length(dw_buffer) % MSG_SIZE), 10);
        //printf("Loop Limit: %zu\n", loop_lim);

        for (int i = 0; i < loop_lim; i++) {
            //printf("[DW] Read input!\n");
            // take input from buffer
            if (evbuffer_remove(dw_buffer, input, MSG_SIZE) < 0)
                printf("[DW] Ooops! A fire!\n");
            // structure data
            circular_buf_msg_structure(input, msg);
            // search if exists in buffer
            found = circular_buf_find(circular_buffer, msg);
            // add in circular buffer if it doesn't exist
            if (!found)
                circular_buf_add(circular_buffer, msg);
        }
        // CREATE MESSAGE
        // TODO: if 5 min have passed create a message and add to buffer
    }
    printf("[DW] Exiting Thread.\n");
    // deallocate memory
    free(msg);
    //exit thread
    pthread_exit(0);

}