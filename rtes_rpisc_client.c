// LIBRARIES
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

/// LIBEVENT
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>

// FILES
#include "rtes_rpisc_p2p.h"
#include "rtes_rpisc_nodeslist.h"
#include "rtes_rpisc_ioworker.h"

/********************************************//**
 *  FUNCTIONS
 ***********************************************/
void *client_main(void *arg) {
    // thread init message
    printf("[CL] Entered Thread Area\n");
    // parse PORT from arguments
    const uintptr_t PORT = (uintptr_t)arg;
    // client loop
    while (1) {
        // for each node in the nodes_list
        for (int node_index = 0; node_index < NODES_NUM; node_index++) {
            // check if node is connected
            if (!node_connected(node_index)) {
                // create socket and try to connect
                if (bufferevent_socket_connect_hostname(node_bev(node_index), NULL, AF_INET, node_ip(node_index), PORT) == 0) {
                    // enable node's bufferevent
                    bufferevent_enable(node_bev(node_index), EV_READ | EV_WRITE);
                }
            }
        }
    }
}