// LIBS load
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

// FILES load
#include "rtes_rpisc_ringbuffer.h"
#include "rtes_rpisc_rwlock.h"

// GLOBAL VARIABLES & CONSTANTS
#define EXAMPLE_BUFFER_SIZE 10

// STRUCTS

// The definition of our circular buffer structure is hidden from the user
// NOTE: write on the report about index being size_t
// NOTE: index and head show on the next item (that's going to be added)
struct circularBuffer {
	uint8_t *buffer_obj;
	rwlock_t lock;
	size_t head;
	size_t index; //id number to keep track of messages when buf size > max
	size_t max; //of the buffer
	bool over_max;
};

// *** PROGRAM START *** //
cbuf circular_buf_init(uint8_t *buffer_obj, size_t size) {
	assert(buffer_obj && size);
	cbuf buffer = malloc(sizeof(circularBuffer));
	assert(buffer);
	buffer->buffer_obj = buffer_obj;
	buffer->max = size;
	circular_buf_reset(buffer);
	assert(circular_buf_empty(buffer));
	return buffer;
}

void circular_buf_add(cbuf buffer, uint8_t data) {
	assert(buffer && buffer->buffer_obj);

	buffer->head = (buffer->head + 1) % buffer->max;
	buffer->index++;

	buffer->buffer_obj[buffer->head] = data;

	if (!buffer->over_max) buffer->over_max = (buffer->head == 0 && buffer->index > 0);
}

int circular_buf_read(cbuf buffer, uint8_t *data, size_t index) {
	assert(buffer && data && buffer->buffer_obj);

	if (circular_buf_empty(buffer)) return -1;

	if (index > buffer->index) return -2;

	if (buffer->over_max) {
		if (index <= buffer->index - EXAMPLE_BUFFER_SIZE) return -3;

		*data = buffer->buffer_obj[index % buffer->max];
		return 0;
	}

	*data = buffer->buffer_obj[index];
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
	return buffer->index + 1;
}

bool circular_buf_empty(cbuf buffer) {
	assert(buffer);
	return (!buffer->over_max && (buffer->head == 0));
}