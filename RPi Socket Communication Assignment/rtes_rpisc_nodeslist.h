#include <stdlib.h>
#include <stdbool.h>

/// Nodes structure
typedef struct node node;

bool node_connected(int node_id);

size_t node_buf_index(int node_id);

struct evbuffer *node_output_buffer(int node_id);

int node_add_to_output_buffer(int node_id, const void *data);

void node_inc_buf_index(int node_id);