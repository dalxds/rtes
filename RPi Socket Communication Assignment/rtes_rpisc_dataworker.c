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
#include <inttypes.h>

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
void create_random_message(cbuf buffer) {
    struct msg *new_msg = malloc(MSG_SIZE);
    struct timeval tv;
    srand(time(NULL));
    //FIXME after debugging
    int random_node_index = rand() % (NODES_NUM + 1);
    new_msg->aem_sender = THIS_AEM;
    //new_msg->aem_receiver = node_aem(random_node_index);
    new_msg->aem_receiver = 5560;
    new_msg->timestamp = (1000000 * tv.tv_sec) + tv.tv_usec;
    strcpy(new_msg->msg_body, "This is node 8607! Hello!");
    circular_buf_add(buffer, new_msg);
}

void *data_worker_main(void *arg) {
    printf("[DW] Entered Thread Area.\n");

    // INIT circular buffer
    printf("[DW] Circular Buffer Initialization\n");
    msg *buffer_obj  = malloc(BUFFER_SIZE * MSG_SIZE);
    cbuf circular_buffer = circular_buf_init(buffer_obj, BUFFER_SIZE);

    // INIT Data Worker ev buffer
    printf("[DW] Data Worker evbuffer Initialization\n");
    extern struct evbuffer *dw_buffer;
    dw_buffer = evbuffer_new();
    if (evbuffer_enable_locking(dw_buffer, NULL) < 0)
        printf("Error on the dw_buffer lock!\n");

    /* DEBUG_SECTION - START */
    //fill the buffer with 10 message
    for (int i = 0; i < 10; i++) {
        create_random_message(circular_buffer);
    }
    print_buffer_status(circular_buffer);
    print_buffer(circular_buffer);
    print_buffer_status(circular_buffer);

    /* DEBUG_SECTION - END */

    // INIT data structures to be used in the endless loop
    struct msg *input_msg = malloc(MSG_SIZE);
    char input_string[MSG_SIZE];    
    struct msg *output_msg = malloc(MSG_SIZE);
    char output_string[MSG_SIZE];
    size_t loop_lim;
    size_t dw_buffer_length;
    int status;
    int found;

    // MAIN LOOP
    while (1) {
        // GET DATA READY FOR OUTPUT
        for (int node_index = 0; node_index < NODES_NUM; node_index++) {
            if (node_connected(node_index)) {
                // check if messages from circular buffer up to the latest
                // have been written to node's buffer
                if (node_cbuf_index(node_index) > circular_buf_index(circular_buffer))
                    continue;
                for (int i = 0; i < 10; i++) {
                    // read message from circular buffer with the index
                    status = circular_buf_read(circular_buffer, output_msg, node_cbuf_index(node_index));
                    printf("Read Status: %d, Node Buffer Index: %zu\n", status, node_cbuf_index(node_index));
                    if (status == -2)
                        break;
                    // destructure data (function in ringbuffer file)
                    circular_buf_msg_destructure(output_msg, output_string);
                    printf("Output: %s\n", output_string);
                    // add to output buffer as string
                    node_add_to_output_buffer(node_index, output_string);
                    // increment node's buffer index
                    node_inc_cbuf_index(node_index);
                }
            }
        }
        // ADD DATA FROM INPUT TO CIRCULAR BUFFER
        dw_buffer_length = evbuffer_get_length(dw_buffer);
        loop_lim = 0;
        if (dw_buffer_length)
            loop_lim = fmin((int)ceil(dw_buffer_length / 287) + 1, 10);

        for (int i = 0; i < loop_lim; i++) {
            //printf("[DW] Read input!\n");
            if (i == 0)
                printf("\nEvBuffer: %zu - Loop Limit: %zu\n",
                       dw_buffer_length,
                       loop_lim);

            // copy input from buffer
            // evbuffer_copyout(dw_buffer, input, MSG_SIZE);
            // printf("\nInput copied: %s\n", input);
            // msg_end = strchr(input, '\n');
            // *msg_end = '\0';
            // input_len = strlen(input);

            // msg_char = evbuffer_pullup(dw_buffer, MSG_SIZE);
            // for (input_len = 0; input_len < MSG_SIZE; input_len++) {
            //     if (msg_char[input_len] == '\n') {
            //         input_len++;
            //         break;
            //     }
            // }
            strcpy(input_string, evbuffer_readln(dw_buffer, NULL , EVBUFFER_EOL_LF));
            printf("\nInput removed: %s\n", input_string);
            // if (evbuffer_remove(dw_buffer, input, sizeof(input)) < 0)
            //     printf("[DW] Ooops! A fire!\n");
            // //DEBUG_SECTION
            // printf("\nInput removed: %s\n", input);
            // structure data
            circular_buf_msg_structure(input_string, input_msg);
            // search if exists in buffer
            found = circular_buf_find(circular_buffer, input_msg);
            printf("\nFOUND? %d\n", found);
            // add in circular buffer if it doesn't exist
            if (!found) {
                circular_buf_add(circular_buffer, input_msg);
                /* DEBUG_SECTION - START */
                printf("\nADDING TO BUFFER...\n");
                print_buffer_status(circular_buffer);
                print_buffer(circular_buffer);
                print_buffer_status(circular_buffer);
                /* DEBUG_SECTION - END */
            }

        }
        // CREATE MESSAGE
    }
    printf("[DW] Exiting Thread.\n");
    // deallocate memory
    free(input_msg);
    free(output_msg);
    //exit thread
    pthread_exit(0);

}