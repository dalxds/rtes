// LIBRARIES
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <sys/time.h>
#include <pthread.h>
#include <inttypes.h>

/// LIBEVENT
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/thread.h>

// FILES
#include "rtes_rpisc_p2p.h"
#include "rtes_rpisc_ioworker.h"
#include "rtes_rpisc_circularbuffer.h"
#include "rtes_rpisc_nodeslist.h"

// GLOBAL VARIABLES & CONSTANTS
#define EVLOOP_NO_EXIT_ON_EMPTY 0x04
#define NODE_BUFFER_READ_COUNT  20
#define DW_BUFFER_READ_COUNT    NODES_NUM * NODE_BUFFER_READ_COUNT

FILE *logfile;

/********************************************//**
 *  HELPER FUNCTIONS
 ***********************************************/
/// @brief  Calculate min
/// @param  x    an integer
/// @param  y    another integer
/// @return int min value of the two integers
int min(int x, int y) {
    return y ^ ((x ^ y) & - (x < y)); 
}

/// @brief  Calculate ceiling of division
/// @param  x    an integer
/// @param  y    another integer
/// @return int the ceil of the two integers' division
int ceiling(int a, int b) {
   return (a + b - 1) / b;
}

/// @brief  Show the read status as string
/// @param  status An integer indicating the status
/// @return str[] A string reading of the status code
char* read_status(int status){
    if (status == 0)
        return "Success";
    else if (status == -1)
        return "Empty Buffer";
    else if (status == -2)
        return "Node Disconnected";
    else if (status == -3)
        return "All Messages Added";
    else if (status == -4)
        return "Message Lost";
    else
        return "";
}

/********************************************//**
 *  LOG FUNCTION
 ***********************************************/
/// @brief  Logs the difference of a current timestamp and a message's (msg) timestamp in a .csv log.
/// @param  *msg A pointer to an msg message
/// @param  mode 1 for input, 2 for output
/// @return void
void log_io(msg *msg, int mode){
    // initialize structures
    struct timeval time;
    uint64_t timestamp;
    /// get timestamp
    gettimeofday(&time, NULL);
    timestamp = (1000000 * time.tv_sec) + time.tv_usec;
    // open log file
    if (mode == 1)
        logfile = fopen("logs/p2p_input_log.csv", "a"); 
    else if (mode == 2)
        logfile = fopen("logs/p2p_output_log.csv", "a");
    else
        err(-2, "Invalid Mode!");
    if(logfile == NULL)
        err(-1, "Could Not Open Log File!");
    // append to file
    fprintf(logfile, "%"PRIu64",%s\n", timestamp, msg->msg_body);
    // close file
    fclose(logfile);
}
/********************************************//**
 *  THREAD FUNCTION
 ***********************************************/
void *data_worker_main(void *arg) {
    // INIT
    /// thread init message
    printf("[DW] Entered Thread Area\n");
    /// circular buffer
    printf("[DW] Circular Buffer Initialization\n");
    msg *buffer_obj  = malloc(BUFFER_SIZE * MAX_MSG_SIZE);
    cbuf circular_buffer = circular_buf_init(buffer_obj, BUFFER_SIZE);
    /// Data Worker evbuffer
    printf("[DW] Data Worker evbuffer Initialization\n");
    extern struct evbuffer *dw_buffer;
    dw_buffer = evbuffer_new();
    if (evbuffer_enable_locking(dw_buffer, NULL) < 0)
        printf("Error on the dw_buffer lock!\n");
    /// data structures
    struct msg *input_msg = malloc(MAX_MSG_SIZE);
    struct msg *output_msg = malloc(MAX_MSG_SIZE);
    char output_string[MAX_MSG_SIZE];
    struct event *msg_gen_event;
    struct timeval timeout;
    int status;
    int found;

    // SEED CIRCULAR BUFFER
    printf("[DW] Message Size: %zu\n", MAX_MSG_SIZE);
    circular_buf_seed(circular_buffer);

    // GENERATE MESSAGE EVERY 5 MINUTES
    // Use timeout event in io worker base
    timeout.tv_sec  = 60 * GEN_FREQ_MIN;
    msg_gen_event = event_new(io_base, -1, EV_TIMEOUT|EV_PERSIST, io_generate_random_message, circular_buffer);
    event_add(msg_gen_event, &timeout);

    // SIGNAL THREAD INITIALIZED
    DATA_WORKER_INIT = true;

    // MAIN LOOP
    while (1) {
        // GET DATA READY FOR OUTPUT
        for (int node_index = 0; node_index < NODES_NUM; node_index++) {
            if (node_connected(node_index)) {
                /// check if messages from circular buffer up to the latest
                /// have been written to node's buffer
                if (node_cbuf_index(node_index) > circular_buf_index(circular_buffer))
                    continue;
                for (int i = 0; i < NODE_BUFFER_READ_COUNT; i++) {
                    /// read message from circular buffer with the index
                    status = circular_buf_read(circular_buffer, output_msg, node_cbuf_index(node_index));
                    printf("[DW] [OUTPUT] Node: %d --- Read Status: %s --- Node Buffer Index: %zu\n",
                        node_index, 
                        read_status(status), 
                        node_cbuf_index(node_index));
                    if (status == -2 || status == -3){
                        break;
                    } else if (status == -4){
                        node_skip_to_index(node_index, 
                            circular_buf_index(circular_buffer),
                            circular_buf_head(circular_buffer));
                        continue;
                    }
                    /// destructure data (function in ringbuffer file)
                    circular_buf_msg_destructure(output_msg, output_string);
                    printf("[DW] [OUTPUT] Node %d --- Message: %s\n", node_index, output_string);
                    /// add to output buffer as string
                    node_add_to_output_buffer(node_index, output_string);
                    /// increment node's buffer index
                    node_inc_cbuf_index(node_index);
                }
            }
        }
        // ADD DATA FROM INPUT TO CIRCULAR BUFFER
        for (int i = 0; i < DW_BUFFER_READ_COUNT && evbuffer_get_length(dw_buffer) > MIN_MSG_SIZE; i++){
            char *input_string = evbuffer_readln(dw_buffer, NULL, EVBUFFER_EOL_LF);
            if (input_string == NULL || strcmp(input_string, "") == 0)
                continue;
            /// printf("[DW] [INPUT] Message: %s\n", input_string);
            /// structure data
            circular_buf_msg_structure(input_string, input_msg);
            /// check if it is generated from this node and log
            log_io(input_msg, 1);
            /// search if exists in buffer
            found = circular_buf_find(circular_buffer, input_msg);
            /// add in circular buffer if it doesn't exist
            if (!found) {
                circular_buf_add(circular_buffer, input_msg);
                printf("[DW] [INPUT] Adding Message to Circular Buffer :: %s\n", input_msg->msg_body);
                print_buffer_status(circular_buffer);
            }
            free(input_string);
        }
    }
}