#include <stdlib.h>
#include <stdbool.h>

#include "rtes_rpisc_p2p.h"

/// Opaque circular buffer structure
typedef struct circularBuffer circularBuffer;

/// Handle type, the way users interact with the API
typedef circularBuffer *cbuf;

/// Pass in a storage buffer and size, returns a circular buffer handle
/// Requires: buffer is not NULL, size > 0
/// Ensures: buffer has been created and is returned in an empty state
cbuf circular_buf_init(msg *buffer, size_t size);

/// Put version 1 continues to add data if the buffer is over_max
/// Old data is overwritten
/// Requires: buffer is valid and created by circular_buf_init
void circular_buf_add(cbuf buffer, msg data);

/// Read a value from the buffer
/// Requires: buffer is valid and created by circular_buf_init
/// Returns 0 on success, -1 if the buffer is empty
int circular_buf_read(cbuf buffer, msg *data, size_t index);

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