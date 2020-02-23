// LIBRARIES
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <sys/time.h>

// FILES
#include "rtes_rpisc_circularbuffer.h"
#include "rtes_rpisc_nodeslist.h"

// STRUCTS
/// The Circular Buffer Structure
// NOTE: index and head show on the next item (that's going to be added)
struct circularBuffer {
    msg         *buffer_obj;
    size_t      index;
    size_t      head;
    size_t      max;
    bool        over_max;
};

// GLOBAL VARIABLES & CONSTANTS
const size_t MAX_MSG_SIZE = sizeof(msg);
const size_t MIN_MSG_SIZE = 17;

/********************************************//**
 *  FUNCTIONS
 ***********************************************/
cbuf circular_buf_init(msg *buffer_obj, size_t size) {
    cbuf buffer = malloc(sizeof(circularBuffer));
    buffer->buffer_obj = buffer_obj;
    buffer->max = size;
    buffer->head = 0;
    buffer->index = 0;
    buffer->over_max = false;
    return buffer;
}

void circular_buf_add(cbuf buffer, msg *msg) {
    buffer->head = (buffer->head + 1) % buffer->max;
    buffer->index++;
    buffer->buffer_obj[buffer->head] = *msg;
    if (!buffer->over_max)
        buffer->over_max = (buffer->head == 0 && buffer->index > 0);
}

int circular_buf_read(cbuf buffer, msg *msg, size_t index) {
    if (circular_buf_is_empty(buffer))
        return -1;
    if (index == 0)
        return -2;
    if (index > buffer->index)
        return -3;
    if (buffer->over_max) {
        if (index <= buffer->index - BUFFER_SIZE)
            return -4;
        *msg = buffer->buffer_obj[index % buffer->max];
        return 0;
    }
    *msg = buffer->buffer_obj[index];
    return 0;
}

void circular_buf_free(cbuf buffer) {
    free(buffer);
}

void circular_buf_reset(cbuf buffer) {
    buffer->head = 0;
    buffer->index = 0;
    buffer->over_max = false;
}

size_t circular_buf_size(cbuf buffer) {
    if(buffer->over_max)
        return buffer->max;
    return buffer->head;
}

size_t circular_buf_index(cbuf buffer) {
    return buffer->index;
}

size_t circular_buf_head(cbuf buffer) {
    return buffer->head;
}

bool circular_buf_is_empty(cbuf buffer) {
    return (buffer->over_max == 0 && (buffer->head == 0));
}

void circular_buf_msg_structure(char str[], msg *msg) {
    char *token = strtok(str, "_");
    char *end;
    msg->aem_sender = strtoul(token, &end, 10);
    token = strtok(NULL, "_");
    msg->aem_receiver = strtoul(token, &end, 10);
    token = strtok(NULL, "_");
    msg->timestamp = strtoull(token, &end, 10);
    token = strtok(NULL, "_");
    strcpy(msg->msg_body, token);
}

void circular_buf_msg_destructure(msg *msg, char str[]) {
    sprintf(str,
            "%"PRIu32"_%"PRIu32"_%"PRIu64"_%s",
            msg->aem_sender,
            msg->aem_receiver,
            msg->timestamp,
            msg->msg_body);
}

int circular_buf_find(cbuf buffer, msg *msg) {
    int status;
    size_t index = circular_buf_index(buffer);
    size_t size = circular_buf_size(buffer);
    struct msg *index_msg = malloc(MAX_MSG_SIZE);
    for (int i = 0; i < size ; i++) {
        status = circular_buf_read(buffer, index_msg, index);
        index--;
        if (status < 0) {
            free(index_msg);
            return -1;
        }
        if (index_msg->timestamp == msg->timestamp) {
            if (index_msg->aem_sender == msg->aem_sender
                    && index_msg->aem_receiver == msg->aem_receiver
                    && !(strcmp(index_msg->msg_body, msg->msg_body))) {
                free(index_msg);
                return 1;
            }
        }
    }
    free(index_msg);
    return 0;
}

void circular_buf_seed(cbuf buffer) {
    // initialize
    int random_node_index = 0;
    struct msg *new_msg = malloc(MAX_MSG_SIZE);
    struct timeval timestamp;
    // initialize randomizer
    srand(time(NULL));
    // run seeder
    for (int i = 0; i < BUFFER_SIZE / 2; i++){
        // sender AEM
        new_msg->aem_sender = THIS_AEM;
        // receiver AEM
        /// choose random node to be receiver
        if (NODES_NUM > 1)  
            random_node_index = rand() % (NODES_NUM - 1);
        /// set receiver
        new_msg->aem_receiver = node_aem(random_node_index);
        // timestamp
        /// get timestamp
        gettimeofday(&timestamp, NULL);
        /// set timestamp
        new_msg->timestamp = (1000000 * timestamp.tv_sec) + timestamp.tv_usec;
        // fill message body
        sprintf(new_msg->msg_body,"%"PRIu32",%"PRIu32",%li\n", THIS_AEM, timestamp.tv_sec, timestamp.tv_usec);
        // add message to circular buffer
        circular_buf_add(buffer, new_msg);
    }
    // print buffer
    print_buffer(buffer);
    print_buffer_status(buffer);
}

void print_buffer_status(cbuf buffer) {
    printf("[DW] [CIRCULAR BUFFER] Status :: Over max: %s, empty: %s, index: %zu, head: %zu\n",
           buffer->over_max ? "true" : "false",
           circular_buf_is_empty(buffer) ? "true" : "false",
           buffer->index,
           buffer->head);
}

void print_buffer(cbuf buffer) {
    printf("*************** CIRCULAR BUFFER PRINT - START ***************\n");
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        printf("Index: %i  --- Value: %" PRIu32 "_%" PRIu32 "_%" PRIu64 "_%s\n",
               i,
               buffer->buffer_obj[i].aem_sender,
               buffer->buffer_obj[i].aem_receiver,
               buffer->buffer_obj[i].timestamp,
               buffer->buffer_obj[i].msg_body);
    }
    printf("**************** CIRCULAR BUFFER PRINT - END ****************\n");
}