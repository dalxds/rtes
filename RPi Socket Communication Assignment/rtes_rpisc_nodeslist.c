// LIBS load
#include <stdlib.h>
#include <stdbool.h>
#include <netinet/in.h>

// LIBEVENT load
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <inttypes.h>

// FILES load
#include "rtes_rpisc_nodeslist.h"
#include "rtes_rpisc_ringbuffer.h"
#include "rtes_rpisc_rwlock.h"

struct node {
	rwlock_t lock;
	char ip[INET_ADDRSTRLEN];
	uint32_t aem;
	size_t buf_index;
	bool connected;
	struct bufferevent *bev;
};

node nodes_list[NODES_NUM];

// TODO create init function
/// add parser here
/// ---create aem inside 

// TODO add rw locks

void nodes_list_parser() {
	char fname[] = "nodes_list.txt";
	FILE *fptr = NULL;
	int i = 0;
	fptr = fopen(fname, "r");

	for (i = 0; i < NODES_NUM; i++) {
		if(fgets(&nodes_list[i].ip[0], INET_ADDRSTRLEN, fptr)) {
			strtok(&nodes_list[i].ip[0], "\n");
		}
	}
	// DEBUGGING 
	// for (i = 0; i < NODES_NUM; i++) {
	// 	if(strcmp(&nodes_list[i].ip[0], "\0")) 
	// 		printf(" %s\n", &nodes_list[i].ip[0]);
	// }
}

bool node_connected(int node_index){
	// block on lock
	return nodes_list[node_index].connected;
}

size_t node_buf_index(int node_index){
	// block on lock
	// add check if still connected (?)
	return nodes_list[node_index].buf_index;
}

void node_inc_buf_index(int node_index){
	// block on lock 
	// add check if still connected (?)
	nodes_list[node_index].buf_index++;
}

int node_add_to_output_buffer(int node_index, char output[]){
	// block on lock
	// add check if still connected (?)
	return evbuffer_add(bufferevent_get_output(nodes_list[node_index].bev), output, MSG_SIZE);
}
