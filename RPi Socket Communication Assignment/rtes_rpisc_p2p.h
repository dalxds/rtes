// LIBRARIES
#include <stdlib.h>
#include <stdbool.h>

// GLOBAL VARIABLES & CONSTANTS
#define NODES_NUM 		5
#define THREADS_NUM     4
#define BUFFER_SIZE     2000
#define GEN_FREQ_MIN 	5

extern volatile bool IO_BASE_STARTED;
extern volatile bool NODES_LIST_INIT_DONE;
extern volatile bool DATA_WORKER_INIT;

extern const uint32_t 	THIS_AEM;