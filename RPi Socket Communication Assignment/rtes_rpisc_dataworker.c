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
	printf("entered DATA WORKER thread area\n");

	// TODO: INIT circular buffer
	msg *buffer_obj  = malloc(NODES_NUM * sizeof(uint8_t));
	cbuf circular_buffer = circular_buf_init(buffer_obj, NODES_NUM);

	// other initializations

	/// loop in nodes list
	/// --for every connected node (done)
	/// 	NOTE: if many nodes a new array with connected nodes would've been created
	/// ---- get output evbuffer and write next 10 messages
	/// -- check data worker buffer evbuffer and read 10 messages 
	/// ---- check if duplicates in circular buffer
	/// ---- add message to circular buffer
	/// 	NOTE: we could make two or more threads but for the number of nodes no such optimization is needed

	while(1) {
		for(int node_id = 0; node_id < NODES_NUM; node_id++){
			if(node_connected(node_id)){
				for (int i = 0; i < 10; i++){
					msg *data;
					circular_buf_read(circular_buffer, data, node_buf_index(node_id));
					node_add_to_output_buffer(node_id, data);
					// TODO: error checking on buf_index increment (has to be < circular buffer index)
					node_inc_buf_index(node_id);
				}
			}
		}
	}

	//exit thread
	pthread_exit(0);
}