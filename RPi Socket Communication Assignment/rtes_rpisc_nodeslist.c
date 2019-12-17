// LIBS load
#include <stdlib.h>
#include <stdbool.h>
#include <netinet/in.h>

// LIBEVENT load
#include <event2/bufferevent.h>
#include <event2/buffer.h>

// FILES load
#include "rtes_rpisc_nodeslist.h"
#include "rtes_rpisc_ringbuffer.h"
#include "rtes_rpisc_rwlock.h"

struct node {
    rwlock_t    lock;
    struct      bufferevent *bev;
    char        ip[INET_ADDRSTRLEN];
    size_t      buf_index;
    uint32_t    node_aem;
    bool        connected;
};

node nodes_list[NODES_NUM];

void nodes_list_init() {
    // init
    int status;
    char fname[] = "nodes_list.txt";
    FILE *fptr = NULL;
    int i = 0;
    fptr = fopen(fname, "r");

    // parse IPs & initiliaze data
    for (i = 0; i < NODES_NUM; i++) {
        nodes_list[i].buf_index = 0;
        nodes_list[i].connected = false;
        if (fgets(&nodes_list[i].ip[0], INET_ADDRSTRLEN, fptr))
            strtok(&nodes_list[i].ip[0], "\n");
    }

    // parse AEMs for IPs
    for (i = 0; i < NODES_NUM; i++) {
        // allocate memory
        char *ip_string = strdup(&nodes_list[i].ip[0]);
        char *aem = malloc(4);

        // remove dots and keep last two parts of string
        char *token = strtok(ip_string, ".");
        char *end;
        token = strtok(NULL, ".");
        token = strtok(NULL, ".");
        strcat(aem, token);
        token = strtok(NULL, ".");
        strcat(aem, token);

        // convert to uint32_t and save to nodes_list item
        nodes_list[i].node_aem = strtoul(aem, &end, 10);

        // free memory
        free(ip_string);
        free(aem);
    }

    // initialize locks
    for (i = 0; i < NODES_NUM; i++) {
        status = rwl_init(&nodes_list[i].lock);
        if (status != 0)
            err_abort (status, "Init rw lock");
    }

    /* DEBUGGING */
    // for (i = 0; i < NODES_NUM; i++) {
    //     if(strcmp(&nodes_list[i].ip[0], "\0"))
    //         printf("IPs: %s\n", &nodes_list[i].ip[0]);
    //         printf("NUM: %u\n", nodes_list[i].node_aem);
    // }
}

bool node_connected(int node_index) {
    int status;
    bool connected;
    status = rwl_readlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read lock");
    connected = nodes_list[node_index].connected;
    status = rwl_readunlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read unlock");
    return connected;
}

size_t node_buf_index(int node_index) {
    int status;
    size_t buf_index;
    status = rwl_readlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read lock");
    if (nodes_list[node_index].connected)
        buf_index = nodes_list[node_index].buf_index;
    status = rwl_readunlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read unlock");
    return buf_index;
}

int node_inc_buf_index(int node_index) {
    int status;
    int status2 = 1;
    status = rwl_writelock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Write lock");
    if (nodes_list[node_index].connected) {
        nodes_list[node_index].buf_index++;
        status2 = 0;
    }
    status = rwl_writeunlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Write unlock");
    return status2;
}

int node_add_to_output_buffer(int node_index, char output[]) {
    int status;
    int status2 = -1;
    status = rwl_readlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read lock");
    if (nodes_list[node_index].connected)
        status2 = evbuffer_add(bufferevent_get_output(nodes_list[node_index].bev), output, MSG_SIZE);
    status = rwl_readunlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read unlock");
    return status2;
}
