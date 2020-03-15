// LIBRARIES
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <sys/time.h>

/// LIBEVENT
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
#include "rtes_rpisc_circularbuffer.h"

// GLOBAL VARIABLES & CONSTANTS
#define EVLOOP_NO_EXIT_ON_EMPTY 0x04

// STRUCTS
struct event_base *io_base;

/********************************************//**
 *  FUNCTIONS
 ***********************************************/
void *io_worker_main(void *arg) {
    // thread init message
    printf("[IO] Entered Thread Area\n");
    // create event base for the main io
    io_base = event_base_new();
    // signal other threads
    IO_BASE_STARTED = true;
    // start the event loop
    event_base_loop(io_base, EVLOOP_NO_EXIT_ON_EMPTY);
}

/********************************************//**
 *  CALLBACK FUNCTIONS
 ***********************************************/
/// @brief  IO Read Callback
/// @param  bufferevent  The bufferevent which triggered the callback
/// @param  arg          Arguments passed to the callback
/// @return void
void io_handle_read(struct bufferevent *bev, void *arg) {
    int node_index = *((int *) arg);
    if (!node_connected(node_index)){
        node_set_connected(node_index);
        printf("[CL] Connected to Node at %s!\n", node_ip(node_index));
    }
    // printf("[IO] Read Callback Triggered!\n");
    if (bufferevent_read_buffer(bev, dw_buffer) < 0)
        printf("Error on reading buffer\n");
}

/// @brief IO Events Callback
/// @param  bufferevent  The bufferevent which triggered the callback
/// @param  evernts      The events that triggered the callback
/// @param  arg          Arguments passed to the callback
/// @return void
void io_handle_events(struct bufferevent *bev, short events, void *arg) {
    int node_index = *((int *) arg);
    if (events & BEV_EVENT_EOF){
        // printf("[IO] ***** Connection closed *****\n");
        // set node disconnected
        node_set_disconnected(node_index);
        // printf("[IO] Node %d is now %s \n", node_index, node_connected(node_index) ? "Connected" : "Disconnected");

    } else if (events & BEV_EVENT_ERROR){
        // printf("[IO] Got an error on the connection to Node %d: %s\n",
        //         node_index,
        //         strerror(errno));
        // set node disconnected
        node_set_disconnected(node_index);
        // printf("[IO] Node %d is now %s \n", node_index, node_connected(node_index) ? "Connected" : "Disconnected");
    } 
    // else if (events & BEV_EVENT_CONNECTED){
    //     printf("[IO] New connection from Node %d\n", node_index);
    // }
}

/// @brief  Create a random message for a node in the nodes_list.
/// @param  buffer  A Circular Buffer handler.
/// @return void
void io_generate_random_message(int fd, short events, void *arg) {
    // initialize
    cbuf buffer = arg;
    int random_node_index = 0;
    struct msg *output_msg = malloc(MAX_MSG_SIZE);
    char output_string[MAX_MSG_SIZE];
    struct timeval timestamp;
    /// initialize randomizer
    srand(time(NULL));
    // get timestamp
    // create message
    /// sender AEM
    output_msg->aem_sender = THIS_AEM;
    /// receiver AEM
    /// choose random node to be receiver
    if (NODES_NUM > 1)  
        random_node_index = rand() % (NODES_NUM - 1);
    output_msg->aem_receiver = node_aem(random_node_index);
    /// get timestamp
    gettimeofday(&timestamp, NULL);
    output_msg->timestamp = (1000000 * timestamp.tv_sec) + timestamp.tv_usec;
    /// fill message body
    sprintf(output_msg->msg_body,"%"PRIu32",%"PRIu32",%li\n", THIS_AEM, timestamp.tv_sec, timestamp.tv_usec);
    /// destructure message
    circular_buf_msg_destructure(output_msg, output_string);
    /// add message to dw_buffer
    evbuffer_add(dw_buffer, output_string, strlen(output_string));
}