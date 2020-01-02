#include <stdint.h>
#include <stdbool.h>

// global variables
#define NODES_NUM       1
#define THREADS_NUM     4
#define BUFFER_SIZE     50

extern volatile bool IO_BASE_STARTED;
extern volatile bool NODES_LIST_INIT_DONE;
extern volatile bool DATA_WORKER_INIT;

