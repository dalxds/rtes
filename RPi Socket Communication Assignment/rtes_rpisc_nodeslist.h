#include <stdlib.h>
#include <stdbool.h>

#include "rtes_rpisc_p2p.h"

/// Nodes structure
typedef struct node node;

void nodes_list_parser();

bool node_connected(int node_index);

size_t node_buf_index(int node_index);

int node_add_to_output_buffer(int node_index, char *data);

void node_inc_buf_index(int node_index);