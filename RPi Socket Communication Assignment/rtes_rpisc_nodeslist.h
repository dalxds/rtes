#include <stdlib.h>
#include <stdbool.h>

void nodes_list_init();

bool node_connected(int node_index);

size_t node_cbuf_index(int node_index);

uint32_t node_aem(int node_index);

struct bufferevent *node_bev(int node_index);

struct sockaddr_in node_addr(int node_index);

int node_inc_cbuf_index(int node_index);

int node_set_bev(int node_index, struct bufferevent *bev);

int node_set_connected(int node_index);

int node_add_to_output_buffer(int node_index, char *data);

size_t node_find_node_index (char ip[]);

extern const size_t ADDRLEN;