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

// FILES load
#include "rtes_rpisc_nodeslist.h"
#include "rtes_rpisc_p2p.h"
#include "rtes_rpisc_rwlock.h"

struct node {
	uint16_t node_id;
	rwlock_t lock;
	char ip[INET_ADDRSTRLEN];
	uint32_t aem;
	size_t buf_index;
	bool connected;
	struct bufferevent *bev;
};

node nodes_list[NODES_NUM];

// TODO: create init function
/// add parser here
/// create aem and node ids here

// TODO: ADD RW LOCKS

bool node_connected(int node_id){
	return nodes_list[node_id].connected;
}

size_t node_buf_index(int node_id){
	return nodes_list[node_id].buf_index;
}

void node_inc_buf_index(int node_id){
	nodes_list[node_id].buf_index++;
}

struct evbuffer *node_input_buffer(int node_id){
	return bufferevent_get_output(nodes_list[node_id].bev);
}

int node_add_to_output_buffer(int node_id, const void *data){
	return evbuffer_add(bufferevent_get_output(nodes_list[node_id].bev), data, sizeof (data));
}
