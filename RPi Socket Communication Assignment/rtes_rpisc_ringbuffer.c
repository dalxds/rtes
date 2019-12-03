#include "rtes_rpisc_ringbuffer.h"

// The hidden definition of our circular buffer structure
struct circular_buf_t {
	uint8_t *buffer;
	size_t head;
	size_t tail;
	size_t max; //of the buffer
	bool full;
	size_t message_id; //overall message_id count
};

// Return a struct
cbuf_handle_t circular_buf_init(uint8_t *buffer, size_t size) {
	assert(buffer && size);
	cbuf_handle_t cbuf = malloc(sizeof(circular_buf_t));
	assert(cbuf);
	cbuf->buffer = buffer;
	cbuf->max = size;
	circular_buf_reset(cbuf);
	assert(circular_buf_empty(cbuf));
	return cbuf;
}

static void advance_pointer(cbuf_handle_t cbuf) {
	assert(cbuf);

	if(cbuf->full) {
		cbuf->tail = (cbuf->tail + 1) % cbuf->max;
	}

	cbuf->head = (cbuf->head + 1) % cbuf->max;
	cbuf->full = (cbuf->head == cbuf->tail);
}

static void retreat_pointer(cbuf_handle_t cbuf) {
	assert(cbuf);
	cbuf->full = false;
	cbuf->tail = (cbuf->tail + 1) % cbuf->max;
}

void circular_buf_reset(cbuf_handle_t cbuf) {
	assert(cbuf);
	cbuf->head = 0;
	cbuf->tail = 0;
	cbuf->full = false;
}

void circular_buf_free(cbuf_handle_t cbuf) {
	assert(cbuf);
	free(cbuf);
}

bool circular_buf_full(cbuf_handle_t cbuf) {
	assert(cbuf);
	return cbuf->full;
}

bool circular_buf_empty(cbuf_handle_t cbuf) {
	assert(cbuf);
	return (!cbuf->full && (cbuf->head == cbuf->tail));
}

size_t circular_buf_capacity(cbuf_handle_t cbuf) {
	assert(cbuf);
	return cbuf->max;
}

size_t circular_buf_size(cbuf_handle_t cbuf) {
	assert(cbuf);
	size_t size = cbuf->max;

	if(!cbuf->full) {
		if(cbuf->head >= cbuf->tail) {
			size = (cbuf->head - cbuf->tail);

		} else {
			size = (cbuf->max + cbuf->head - cbuf->tail);
		}
	}

	return size;
}

size_t circular_buf_count(cbuf_handle_t cbuf) {
	assert(cbuf);
	return cbuf->message_id;
}

void circular_buf_put(cbuf_handle_t cbuf, uint8_t data) {
	assert(cbuf && cbuf->buffer);
	cbuf->buffer[cbuf->head] = data;
	cbuf->message_id++;
	advance_pointer(cbuf);
}

int circular_buf_get(cbuf_handle_t cbuf, uint8_t *data_recv) {
	assert(cbuf && data_recv && cbuf->buffer);
	int r = -1;

	if(!circular_buf_empty(cbuf)) {
		*data_recv = cbuf->buffer[cbuf->tail];
		retreat_pointer(cbuf);
		r = 0;
	}

	return r;
}