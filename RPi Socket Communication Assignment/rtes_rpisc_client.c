// LIBS load
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <pthread.h>

// LIBEVENT load
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>

// FILES load
#include "rtes_rpisc_p2p.h"
#include "rtes_rpisc_nodeslist.h"
#include "rtes_rpisc_ioworker.h"

// CONSTANTS

// STRUCTS

// *** FUNCTIONS - START *** //

void *client_main(void *arg) {
    printf("[CL] Entered Thread Area\n");
    const uintptr_t PORT = (uintptr_t)arg;
    while (1) {
        for (int node_index = 0; node_index < NODES_NUM; node_index++) {
            if (!node_connected(node_index)) {
                // FIXME be careful! may block indefinately
                // FIXME be careful! handling when no one connected
                if (bufferevent_socket_connect_hostname(node_bev(node_index), NULL, AF_INET, node_ip(node_index), PORT) == 0) {
                    printf("[CL] Server Found!\n");
                    bufferevent_enable(node_bev(node_index), EV_READ | EV_WRITE);
                    printf("[CL] Bufferevent enabled!\n");
                    node_set_connected(node_index);
                }
            }
        }
    }

    pthread_exit(0);
}

