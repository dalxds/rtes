#include <stdlib.h>
#include <stdbool.h>

#include "rtes_rpisc_p2p.h"

/// Opaque circular buffer structure
typedef struct circularBuffer circularBuffer;

/// Handle type, the way users interact with the API
typedef circularBuffer *cbuf;

/// Messages structure
typedef struct msg {
    uint32_t    aem_sender;
    uint32_t    aem_receiver;
    uint64_t    timestamp;
    char        msg_body[256];
    uint32_t    recv_from;  
} msg;

extern const size_t MSG_SIZE;

/// Pass in a storage buffer and size, returns a circular buffer handle
/// Requires: buffer is not NULL, size > 0
/// Ensures: buffer has been created and is returned in an empty state
cbuf circular_buf_init(msg *buffer, size_t size);

/// Adds data to the buffer
/// Old data is overwritten
/// Requires: buffer is valid and created by circular_buf_init
void circular_buf_add(cbuf buffer, msg *msg);

/// Read a value from the buffer
/// Requires: buffer is valid and created by circular_buf_init
/// Returns 0 on success, -1 if the buffer is empty
int circular_buf_read(cbuf buffer, msg *msg, size_t index);

/// Free a circular buffer structure
/// Requires: buffer is valid and created by circular_buf_init
/// Does not free data buffer; owner is responsible for that
void circular_buf_free(cbuf buffer);

/// Reset the circular buffer to empty, head == tail. Data not cleared
/// Requires: buffer is valid and created by circular_buf_init
void circular_buf_reset(cbuf buffer);

/// Check the current size of the buffer
/// Requires: buffer is valid and created by circular_buf_init
/// Returns the current number of the index (all messages written to buffer)
size_t circular_buf_size(cbuf buffer);

/// Check the current value of the index
/// Requires: buffer is valid and created by circular_buf_init
/// Returns the current number of the index (all messages written to buffer)
size_t circular_buf_index(cbuf buffer);

/// Checks if the buffer is empty
/// Requires: buffer is valid and created by circular_buf_init
/// Returns true if the buffer is empty
bool circular_buf_empty(cbuf buffer);

/// Checks if the buffer is over_max
/// Requires: buffer is valid and created by circular_buf_init
/// Returns true if the buffer is over_max
bool circular_buf_over_max(cbuf buffer);

/* HELPER FUNCTIONS*/
/// Converts a String to msg object (structuring)
/// Requires: a String and a pointer to an allocated msg object to place the result
void circular_buf_msg_structure(char str[], msg *msg);

/// Converts an msg object to a String using "_" as delimiter (destructuring)
/// Requires: a pointer to an msg object and a string to place the result
void circular_buf_msg_destructure(msg *msg, char str[]);

/// Searches in the circular buffer if the msg object exists
/// Requires: a cbuf buffer and an msg object
/// Returns 1 if element found, 0 on success with no found, -1 if can't read
int circular_buf_find(cbuf buffer, msg *msg);