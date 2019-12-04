#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

/// Opaque circular buffer structure
typedef struct circular_buffer circular_buffer;

/// Handle type, the way users interact with the API
typedef circular_buffer *cbuf;

/// Pass in a storage buffer and size, returns a circular buffer handle
/// Requires: buffer is not NULL, size > 0
/// Ensures: buffer has been created and is returned in an empty state
cbuf circular_buf_init(uint8_t *buffer, size_t size);

/// Put version 1 continues to add data if the buffer is over_max
/// Old data is overwritten
/// Requires: buffer is valid and created by circular_buf_init
void circular_buf_add(cbuf buffer, uint8_t data);

/// Read a value from the buffer
/// Requires: buffer is valid and created by circular_buf_init
/// Returns 0 on success, -1 if the buffer is empty
int circular_buf_read(cbuf buffer, uint8_t *data, size_t index);

/// Free a circular buffer structure
/// Requires: buffer is valid and created by circular_buf_init
/// Does not free data buffer; owner is responsible for that
void circular_buf_free(cbuf buffer);

/// Reset the circular buffer to empty, head == tail. Data not cleared
/// Requires: buffer is valid and created by circular_buf_init
void circular_buf_reset(cbuf buffer);

/// Check the current value of the index
/// Requires: buffer is valid and created by circular_buf_init
/// Returns the current number of the index (all messages written to buffer)
size_t circular_buf_size(cbuf buffer);

/// Checks if the buffer is empty
/// Requires: buffer is valid and created by circular_buf_init
/// Returns true if the buffer is empty
bool circular_buf_empty(cbuf buffer);

/// Checks if the buffer is over_max
/// Requires: buffer is valid and created by circular_buf_init
/// Returns true if the buffer is over_max
bool circular_buf_over_max(cbuf buffer);

// The definition of our circular buffer structure is hidden from the user
// NOTE: write on the report about index being size_t
// NOTE: index and head show on the next item (that's going to be added)
struct circular_buffer {
	uint8_t *buffer_obj;
	size_t head;
	size_t index; //id number to keep track of messages when buf size > max
	size_t max; //of the buffer
	bool over_max;
};

#define EXAMPLE_BUFFER_SIZE 10

cbuf circular_buf_init(uint8_t *buffer_obj, size_t size) {
	assert(buffer_obj && size);
	cbuf buffer = malloc(sizeof(circular_buffer));
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

	if(!buffer->over_max) buffer->over_max = (buffer->head == 0 && buffer->index > 0);
}

int circular_buf_read(cbuf buffer, uint8_t *data, size_t index) {
	assert(buffer && data && buffer->buffer_obj);

	if(circular_buf_empty(buffer)) return -1;

	if(index > buffer->index) return -2;

	if(buffer->over_max) {
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


void print_buffer_status(cbuf buffer) {
	printf("\nSTATUS === Over max: %d, empty: %d, index: %zu, head: %zu ===\n",
				 buffer->over_max,
				 circular_buf_empty(buffer),
				 buffer->index,
				 buffer->head);
}

int main(int argc, char const *argv[]) {
	uint8_t *buffer_obj  = malloc(EXAMPLE_BUFFER_SIZE * sizeof(uint8_t));
	printf("\n=== C Circular Buffer Check ===\n");
	cbuf buffer = circular_buf_init(buffer_obj, EXAMPLE_BUFFER_SIZE);
	printf("Buffer initialized.\n");
	print_buffer_status(buffer);

	printf("\n******Filling buffer******\n");

	for(uint8_t i = 0; i < (EXAMPLE_BUFFER_SIZE - 1); i++) {
		circular_buf_add(buffer, i * 5);
		printf("value %u *** index %zu *** head %zu \n",
					 i * 5,
					 buffer->index,
					 buffer->head);
	}

	printf("\n******Printing buffer******\n");

	for(uint8_t i = 0; i < (EXAMPLE_BUFFER_SIZE); i++) {
		printf("value %i *** pseudoindex %i\n",
					 buffer->buffer_obj[i],
					 i);
	}

	print_buffer_status(buffer);
	printf("\n******Reading values******\n");
	int input;

	while(1) {
		printf("\nEnter Index:\n");
		if (scanf("%i", &input) != 1)
            break;
		uint8_t data;
		int data_return = circular_buf_read(buffer, &data, input);
		printf("\nRead Function Status --> %i <--\n", data_return );

		if(!data_return)
			printf("Data: %u\n", data);
	}

	printf("\n******EXIT******\n");
	return 0;
}