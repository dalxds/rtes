// LIBRARIES
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/// LIBEVENT
#include <event2/bufferevent.h>
#include <event2/buffer.h>

// FILES
#include "rtes_rpisc_nodeslist.h"
#include "rtes_rpisc_circularbuffer.h"
#include "rtes_rpisc_rwlock.h"
#include "rtes_rpisc_ioworker.h"

// STRUCTS
/// Node Structure
typedef struct node {
    rwlock_t    lock;
    struct      bufferevent *bev;
    char        ip[INET_ADDRSTRLEN];
    int         node_index;
    size_t      cbuf_index;
    uint32_t    node_aem;
    bool        connected;
} node;

/// Nodes List (array of NODE_NUM node)
struct node nodes_list[NODES_NUM];

/********************************************//**
 *  FUNCTIONS
 ***********************************************/
void nodes_list_init() {
    // init
    int status;

    // Nodes List
    char fname[] = "nodes_list.txt";
    FILE *fptr = NULL;
    int i = 0;
    fptr = fopen(fname, "r");
    if(fptr == NULL)
        err_abort(-1, "Error With File!");

    // parse IPs & initiliaze data
    for (i = 0; i < NODES_NUM; i++) {
        // NODE INDEX
        nodes_list[i].node_index = i;

        // CBUF INDEX
        //NOTE messages in buffer start from 1
        nodes_list[i].cbuf_index = 1;

        // CONNECTION STATUS
        nodes_list[i].connected = false;

        // IP
        /// struct
        struct sockaddr_in addr;
        /// parse ip from file
        if (fgets(&nodes_list[i].ip[0], INET_ADDRSTRLEN, fptr))
            strtok(&nodes_list[i].ip[0], "\n");
        if(strcmp(nodes_list[i].ip, "") == 0)
            err_abort(-1, "Parse IP Error! Check NODES_NUM !?");
        /// save copy of ip
        char *ip_string = strdup(&nodes_list[i].ip[0]);
        /// convert 2 times to save in required format
        if(inet_aton(ip_string, &addr.sin_addr) == 0)
            err_abort(-1, "Parse IP Error! aton conversion");
        strcpy(nodes_list[i].ip, inet_ntoa(addr.sin_addr)); 

        // BUFFEREVENT
        /// set bufferevent
        nodes_list[i].bev = malloc(sizeof(struct bufferevent *));
        nodes_list[i].bev = bufferevent_socket_new(io_base, -1, BEV_OPT_THREADSAFE);
        /// set bufferevent's callbacks
        bufferevent_setcb(nodes_list[i].bev, io_handle_read, NULL, io_handle_events, &nodes_list[i].node_index);

        // AEMS (parse from IPs)
        /// allocate memory
        char *aem = malloc(4);
        /// remove dots and keep last two parts of string
        char *token = strtok(ip_string, ".");
        char *end;
        token = strtok(NULL, ".");
        token = strtok(NULL, ".");
        strcat(aem, token);
        token = strtok(NULL, ".");
        strcat(aem, token);

        /// convert to uint32_t and save to nodes_list item
        nodes_list[i].node_aem = strtoul(aem, &end, 10);

        /// free memory
        free(ip_string);
        free(aem);
    
        // initialize locks
        status = rwl_init(&nodes_list[i].lock);
        // printf("Lock %d initialized with status code %d\n", i, status);
        if (status != 0)
            err_abort (status, "Init RW Lock");
    }
    
    printf("********** NODES LIST PRINT - START **********\n");
    for (i = 0; i < NODES_NUM; i++) {
        if(strcmp(nodes_list[i].ip, "\0")){
            printf("=== Node %d ===\n", i);
            printf("IP: %s\n", nodes_list[i].ip);
            printf("AEM: %u\n", nodes_list[i].node_aem);
        }
    }
    printf("*********** NODES LIST PRINT - END ***********\n");

    // signal DONE
    NODES_LIST_INIT_DONE = true;
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

size_t node_cbuf_index(int node_index) {
    int status;
    size_t buf_index = 0;
    status = rwl_readlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read lock");
    if (nodes_list[node_index].connected)
        buf_index = nodes_list[node_index].cbuf_index;
    status = rwl_readunlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read unlock");
    return buf_index;
}

char* node_ip(int node_index) {
    int status;
    char* ip;
    status = rwl_readlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read lock");
    ip = nodes_list[node_index].ip;
    status = rwl_readunlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read unlock");
    return ip;
}

uint32_t node_aem(int node_index) {
    int status;
    uint32_t node_aem;
    status = rwl_readlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read lock");
        node_aem = nodes_list[node_index].node_aem;
    status = rwl_readunlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read unlock");
    return node_aem;
}

struct bufferevent *node_bev(int node_index) {
    int status;
    struct bufferevent *bev;
    status = rwl_readlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read lock");
    bev = nodes_list[node_index].bev;
    status = rwl_readunlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read unlock");
    return bev;
}

int node_inc_cbuf_index(int node_index) {
    int status;
    int status2 = -1;
    status = rwl_writelock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Write lock");
    if (nodes_list[node_index].connected) {
        nodes_list[node_index].cbuf_index++;
        status2 = 0;
    }
    status = rwl_writeunlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Write unlock");
    return status2;
}

int node_skip_to_index(int node_index, size_t index, size_t head) {
    int status;
    int status2 = -1;
    status = rwl_writelock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Write lock");
    if (nodes_list[node_index].connected) {
        nodes_list[node_index].cbuf_index = index - head;
        status2 = 0;
    }
    status = rwl_writeunlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Write unlock");
    return status2;
}

int node_set_connected(int node_index) {
    int status;
    int status2 = -1;
    status = rwl_writelock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Write lock");
    nodes_list[node_index].connected = true;
    status2 = 0;
    status = rwl_writeunlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Write unlock");
    return status2;
}

int node_set_disconnected(int node_index) {
    int status;
    int status2 = -1;
    status = rwl_writelock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Write lock");
    nodes_list[node_index].connected = false;
    status2 = 0;
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
        status2 = evbuffer_add(bufferevent_get_output(nodes_list[node_index].bev), output, strlen(output));
    status = rwl_readunlock(&nodes_list[node_index].lock);
    if (status != 0)
        err_abort (status, "Read unlock");
    return status2;
}

size_t node_find_node_index_by_ip(char ip[]) {
    int status;
    int node_index = -1;
    for (int i = 0; i < NODES_NUM; i++) {
        status = rwl_readlock(&nodes_list[i].lock);
        if (status != 0)
            err_abort (status, "Read lock");
        if (!strcmp(ip, nodes_list[i].ip))
            node_index = nodes_list[i].node_index;
        status = rwl_readunlock(&nodes_list[i].lock);
        if (status != 0)
            err_abort (status, "Read unlock");
    }
    return node_index;
}
