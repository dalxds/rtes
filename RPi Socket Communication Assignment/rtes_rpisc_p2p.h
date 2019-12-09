#include <stdlib.h>

// global variables
#define NODES_NUM 		10
#define THREADS_NUM		4

/// Messages structure
typedef struct msg {
	uint32_t      aem_sender;
	uint32_t      aem_receiver;
	uint64_t      timestamp;
	char          msg_body[256];
} msg;