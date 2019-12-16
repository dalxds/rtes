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
    printf("[DW] Circular Buffer Initialization");
    msg *buffer_obj  = malloc(NODES_NUM * MSG_SIZE);
    cbuf circular_buffer = circular_buf_init(buffer_obj, NODES_NUM);

    // INIT Data Worker ev buffer
    printf("[DW] Data Worker evbuffer Initialization\n");
    extern struct evbuffer *dw_buffer;
    dw_buffer = evbuffer_new();
    if (evbuffer_enable_locking(dw_buffer, NULL) < 0)
        printf("Error on the dw_buffer lock! \n");

    // INIT data structures to be used in the endless loop
    struct msg *msg = malloc(MSG_SIZE);
    char input[MSG_SIZE];
    char output[MSG_SIZE];
    int found;

    while (1) {
        // GET DATA READY FOR OUTPUT
        /// --for every connected node (done)
        /// NOTE if many nodes a new array with connected nodes would've been created
        /// ---- get output evbuffer and write next 10 messages (done)

        // initialize structures to hold the data
        for (int node_index = 0; node_index < NODES_NUM; node_index++) {
            if (node_connected(node_index)) {
                // check if messages from circular buffer up to the latest have been written to node's buffer
                if (node_buf_index(node_index) > circular_buf_index(circular_buffer))
                    break;
                for (int i = 0; i < 10; i++) {
                    // read message from circular buffer with the index
                    circular_buf_read(circular_buffer, msg, node_buf_index(node_index));
                    // destructure data (function in ringbuffer file)
                    circular_buf_msg_destructure(msg, output);
                    // add to output buffer as string
                    node_add_to_output_buffer(node_index, output);
                    // increment node's buffer index
                    node_inc_buf_index(node_index);
                }
            }
        }
        // ADD DATA FROM INPUT TO CIRCULAR BUFFER
        /// ---- structure message (done)
        /// ---- check if duplicates in circular buffer (done)
        /// ---- add message to circular buffer (done)
        /// NOTE we could make two or more threads but for the number of nodes no such optimization is needed
        for (int i = 0; i < 10; i++) {
            // take input from buffer
            evbuffer_remove(dw_buffer, input, MSG_SIZE);
            // structure data
            circular_buf_msg_structure(input, msg);
            // search if exists in buffer
            found = circular_buf_find(circular_buffer, msg);
            // add in circular buffer if it doesn't exist
            if (!found)
                circular_buf_add(circular_buffer, msg);
        }
        // CREATE MESSAGE
    }
    printf("[DW] Exiting Thread.\n");
    // deallocate memory
    free(msg);
    //exit thread
    pthread_exit(0);

}