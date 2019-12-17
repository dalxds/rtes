#include <stdlib.h>
#include <stdbool.h>

/// Nodes structure
typedef struct node node;

void nodes_list_init();

bool node_connected(int node_index);

size_t node_buf_index(int node_index);

int node_add_to_output_buffer(int node_index, char *data);

int node_inc_buf_index(int node_index);