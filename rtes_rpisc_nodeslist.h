// LIBRARIES
#include <stdlib.h>
#include <stdbool.h>

/********************************************//**
 *  FUNCTIONS
 ***********************************************/
/// @brief Initialize the nodes_list structure.
/// Parses the nodes_list.txt file
/// For every ip in the node_list.txt file 
/// it creates a node struct in the nodes_list array which contains:
///  - An rwlock
///  - A bufferevent for the IO
///  - The node's ip as string and the node's AEM as a number
///  - The node's conenction status
/// @return void
void nodes_list_init();

/// @brief 	Return whether a node is connected or not.
/// @param 	node_index 	The node's index in the nodes_list array.
/// @return true 		The node is connected.
/// @return false 		The node is disconnected.
bool node_connected(int node_index);

/// @brief 	Return the node's message index for the Circular Buffer.
/// The cbuf_index indicates the index of the last message that was sent to the node.
/// @param 	node_index 	The node's index in the nodes_list array.
/// @return size_t 		The node's Circular Buffer index.
size_t node_cbuf_index(int node_index);

/// @brief 	Return the node's ip.
/// @param 	node_index 	The node's index in the nodes_list array.
/// @return char[] 		The node's ip.
char* node_ip(int node_index);

/// @brief 	Return the node's aem.
/// @param 	node_index 	The node's index in the nodes_list array.
/// @return uint32_t	The node's aem.
uint32_t node_aem(int node_index);

/// @brief 	Return the node's bufferevent.
/// @param 	node_index 	The node's index in the nodes_list array.
/// @return uint32_t	The node's bufferevent.
struct bufferevent *node_bev(int node_index);

/// @brief 	Increment node's cbuf index by one.
/// @param 	node_index 	The node's index in the nodes_list array.
/// @return 0 			Success
/// @return -1 			Error
int node_inc_cbuf_index(int node_index);

/// @brief 	Skips node's cbuf_index to the given index.
/// Used when there is a big difference between the node's cbuf_index
/// and the Circular Buffer's index, so that the node can catch up.
/// @param 	node_index 	The node's index in the nodes_list array.
/// @param	index 		The index to skip to.
/// @return 0 			Success
/// @return -1 			Error
int node_skip_to_index(int node_index, size_t index, size_t head);

/// @brief 	Set the node's connected status to connected (true).
/// @param 	node_index 	The node's index in the nodes_list array.
/// @return 0 			Success
/// @return -1 			Error
int node_set_connected(int node_index);

/// @brief 	Set the node's connected status to disconnected (false).
/// @param 	node_index 	The node's index in the nodes_list array.
/// @return 0 			Success
/// @return -1 			Error
int node_set_disconnected(int node_index);

/// @brief 	Add a string to the node's output buffer.
/// @param 	node_index 	The node's index in the nodes_list array.
/// @param 	output[] 	The string to be added to the node's output bufferevent.
/// @return 0 			Success
/// @return -1 			Error
int node_add_to_output_buffer(int node_index, char output[]);

/// @brief 	Return the node's index for a giver ip.
/// @param 	ip[] 	The ip of the node to be foung, in string format.
/// @return -1 		If no node was found with the given ip.
/// @return size_t 	The node's index		
size_t node_find_node_index_by_ip(char ip[]);