// LIBS load
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>

// FILES load
#include "rtes_rpisc_ringbuffer.h"

// GLOBAL VARIABLES & CONSTANTS
const size_t MSG_SIZE = sizeof(msg);
const uint32_t THIS_AEM = 8607;

// STRUCTS

// The definition of our circular buffer structure is hidden from the user
// NOTE: write on the report about index being size_t
// NOTE: index and head show on the next item (that's going to be added)
struct circularBuffer {
    msg         *buffer_obj;
    size_t      head;
    size_t      index; //id number to keep track of messages when buf size > max
    size_t      max; //of the buffer
    bool        over_max;
};

// *** PROGRAM START *** //
cbuf circular_buf_init(msg *buffer_obj, size_t size) {
    assert(buffer_obj && size);
    // DEBUG_SECTION -- START
    printf("SIZEOF BUFFER: %zu\n", sizeof(circularBuffer));
    printf("SIZEOF MSG %zu\n", MSG_SIZE);
    // DEBUG_SECTION -- END
    cbuf buffer = malloc(sizeof(circularBuffer));
    assert(buffer);
    buffer->buffer_obj = buffer_obj;
    buffer->max = size;
    buffer->head = 0;
    buffer->index = 0;
    buffer->over_max = false;
    circular_buf_reset(buffer);
    assert(circular_buf_empty(buffer));
    return buffer;
}

void circular_buf_add(cbuf buffer, msg *msg) {
    assert(buffer && buffer->buffer_obj);
    buffer->head = (buffer->head + 1) % buffer->max;
    buffer->index++;
    buffer->buffer_obj[buffer->head] = *msg;

    if (!buffer->over_max)
        buffer->over_max = (buffer->head == 0 && buffer->index > 0);
}

int circular_buf_read(cbuf buffer, msg *msg, size_t index) {
    assert(buffer && msg && buffer->buffer_obj);

    if (circular_buf_empty(buffer))
        return -1;

    if (index > buffer->index)
        return -2;

    if (buffer->over_max) {
        if (index <= buffer->index - BUFFER_SIZE)
            return -3;

        *msg = buffer->buffer_obj[index % buffer->max];
        return 0;
    }

    *msg = buffer->buffer_obj[index];
    return 0;
}

void circular_buf_free(cbuf buffer) {
    assert(buffer);
    free(buffer);
}

void circular_buf_reset(cbuf buffer) {
    assert(buffer);
    buffer->head = 0;
    buffer->index = 0;
    buffer->over_max = false;
}

size_t circular_buf_size(cbuf buffer) {
    assert(buffer);
    size_t size = buffer->max;

    if (!buffer->over_max)
        size = buffer-> head;

    return size;
}

size_t circular_buf_index(cbuf buffer) {
    assert(buffer);
    return buffer->index;
}

bool circular_buf_empty(cbuf buffer) {
    assert(buffer);
    return (!buffer->over_max && (buffer->head == 0));
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
            "%"PRIu32"_%"PRIu32"_%"PRIu64"_%s\n",
            msg->aem_sender,
            msg->aem_receiver,
            msg->timestamp,
            msg->msg_body);
}

//NOTE: a more optimized search approach is not needed due to the number of the buf elements
int circular_buf_find(cbuf buffer, msg *msg) {
    int status;
    size_t index = circular_buf_size(buffer);
    struct msg *index_msg = malloc(MSG_SIZE);
    for (int i = 0; i < index; i++) {
        status = circular_buf_read(buffer, index_msg, index);
        if (status < 0){
            free(index_msg);
            return -1;
        }
        if (index_msg->timestamp == msg->timestamp){
            if (index_msg->aem_sender == msg->aem_sender
                    && index_msg->aem_receiver == msg->aem_receiver
                    && strcmp(index_msg->msg_body, msg->msg_body)){
                free(index_msg);
                return 1;
            }
        }
    }
    free(index_msg);
    return 0;
}

/* DEBUG_SECTION DEBUG FUNCTIONS - START */
void print_buffer_status(cbuf buffer) {
    printf("\n\nSTATUS === Over max: %d, empty: %d, index: %zu, head: %zu ===\n\n",
           buffer->over_max,
           circular_buf_empty(buffer),
           buffer->index,
           buffer->head);
}

void print_buffer(cbuf buffer) {
    for (int i = 0; i < (BUFFER_SIZE); ++i) {
        printf("Index: %i  --- Value: %" PRIu32 "_%" PRIu32 "_%" PRIu64 "_%s\n",
               i,
               buffer->buffer_obj[i].aem_sender,
               buffer->buffer_obj[i].aem_receiver,
               buffer->buffer_obj[i].timestamp,
               buffer->buffer_obj[i].msg_body);
    }

}
/* DEBUG_SECTION DEBUG FUNCTIONS - END */
