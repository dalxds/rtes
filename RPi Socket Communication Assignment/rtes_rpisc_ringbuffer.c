// LIBS load
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>

// FILES load
#include "rtes_rpisc_ringbuffer.h"
#include "rtes_rpisc_rwlock.h"

// GLOBAL VARIABLES & CONSTANTS

// STRUCTS

// The definition of our circular buffer structure is hidden from the user
// NOTE: write on the report about index being size_t
// NOTE: index and head show on the next item (that's going to be added)
struct circularBuffer {
    msg *buffer_obj;
    rwlock_t lock;
    size_t head;
    size_t index; //id number to keep track of messages when buf size > max
    size_t max; //of the buffer
    bool over_max;
};

// *** PROGRAM START *** //
cbuf circular_buf_init(msg *buffer_obj, size_t size) {
    assert(buffer_obj && size);
    cbuf buffer = malloc(sizeof(circularBuffer));
    assert(buffer);
    buffer->buffer_obj = buffer_obj;
    buffer->max = size;
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
            "%"PRIu32"_%"PRIu32"_%"PRIu64"_%s",
            msg->aem_sender,
            msg->aem_receiver,
            msg->timestamp,
            msg->msg_body);
}

//NOTE: a more optimized search approach is not needed due to the number of the buf elements
int circular_buf_find(cbuf buffer, msg *msg) {
    assert(buffer && msg);
    int status;
    int index = circular_buf_index(buffer) - circular_buf_size(buffer);
    struct msg *index_msg = malloc(MSG_SIZE);
    for (int i = index; i < circular_buf_size(buffer); i++) {
        status = circular_buf_read(buffer, index_msg, index);
        if (!status) return -1;
        if (index_msg->timestamp == msg->timestamp) {
            if (index_msg->aem_sender == msg->aem_sender
                    && index_msg->aem_receiver == msg->aem_receiver
                    && strcmp(index_msg->msg_body, msg->msg_body))
                return 1;
        }
    }
    return 0;
}
