// LIBRARIES
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

/// LIBEVENT
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/thread.h>

// FILES
#include "rtes_rpisc_p2p.h"
#include "rtes_rpisc_server.h"
#include "rtes_rpisc_ioworker.h"
#include "rtes_rpisc_nodeslist.h"

void server_on_accept (struct evconnlistener *listener, evutil_socket_t accepted_fd,
                       struct sockaddr *address, int socklen, void *ctx) {
    // init variables
    int node_index;
    int status;

    // parse IP of accepted socket
    struct sockaddr_in *node_addr = (struct sockaddr_in *)address;
    char *node_ip = inet_ntoa(node_addr->sin_addr);

    // find node_index by IP
    node_index = node_find_node_index_by_ip(node_ip);
    if (node_index < 0){
        printf("[SE] node_index couldn't be retrieved! Connection Closed!\n");
        close(accepted_fd);
        return;
    }

    // reject if node connected
    if (node_connected(node_index)) {
        printf("[SE] Connection from %s rejected because node already connected!\n", node_ip);
        close(accepted_fd);
        return;
    }

    // add socket to bufferevent
    bufferevent_setfd(node_bev(node_index), accepted_fd);

    // enable bufferevent
    bufferevent_enable(node_bev(node_index), EV_READ | EV_WRITE);

    // signal node connected
    status = node_set_connected(node_index);

    // print message
    printf("[SE] Accepted connection from %s | Node Index: %d | Status: %s\n", 
        node_ip, 
        node_index, 
        status == 0 ? "Connected" : "Disconnected");
}

/********************************************//**
 *  THREAD FUNCTION
 ***********************************************/
void *server_main(void *arg) {
    // thread init message
    printf("[SE] Entered Thread Area\n");

    // initialize structures
    struct event_base *server_base;
    struct sockaddr_in sin;
    struct evconnlistener *listener;

    // initialize libevent server base
    server_base = event_base_new();
    if (!server_base)
        err(1, "[SE] Server Base Initialization Failed!");

    // set up listening address
    /// clear sockaddr before using
    memset(&sin, 0, sizeof(sin));
    /// set up listening socket's family
    sin.sin_family = AF_INET;
    /// set up listening address
    sin.sin_addr.s_addr = INADDR_ANY;
    /// set up listening port
    sin.sin_port = htons((uintptr_t)arg);

    // listener
    listener = evconnlistener_new_bind(server_base, server_on_accept, NULL,
                                       LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                       (struct sockaddr*)&sin, sizeof(sin));
    if (!listener)
        err(1, "[SE] Couldn't create listener!");

    // start the event loop
    printf("[SE] Server Base Dispatched!\n");
    event_base_dispatch(server_base);
}