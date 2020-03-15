// FILES
#include "rtes_rpisc_p2p.h"

// STRUCTS
/// Opaque circular buffer structure
typedef struct circularBuffer circularBuffer;

/// Handle type, the way users interact with the API
typedef circularBuffer *cbuf;

/// Messages structure
typedef struct msg {
    uint32_t    aem_sender;
    uint32_t    aem_receiver;
    uint64_t    timestamp;
    char        msg_body[256 + sizeof('\n')];
} msg;

// GLOBAL VARIABLES & CONSTANTS
extern const size_t 	MAX_MSG_SIZE;
extern const size_t 	MIN_MSG_SIZE;
// extern const uint32_t 	THIS_AEM;

/********************************************//**
 *  MAIN API FUNCTIONS
 ***********************************************/
/// @brief  Circular Buffer Initialization
/// @param  buffer_obj  A Buffer Object
/// @param  size        The number of items the Circular Buffer will hold.
/// @return buffer      A Circular Buffer object handler.
cbuf circular_buf_init(msg *buffer, size_t size);

/// @brief  Add an item to the Circular Buffer.
/// @param  buffer  A Circular Buffer handler.
/// @param  msg     The item to be added to the Circular Buffer.
/// @return void
void circular_buf_add(cbuf buffer, msg *msg);

/// @brief  Read an item from the Circular Buffer.
/// @param  buffer  A Circular Buffer handler.
/// @param  msg     An msg struct where the message will be written.
/// @param  index   The index of the item to be read.
/// @return 0       Successful Read!
/// @return -1      The Circular Buffer is empty.
/// @return -2      Node has disconnected.
/// @return -3      The given index is bigger than the current Circular Buffer index.
/// @return -4      The requested message has been overwritten (lost) in the Circular Buffer.
int circular_buf_read(cbuf buffer, msg *msg, size_t index);

/// @brief 	Free the buffer structure.
/// @param  buffer  A Circular Buffer handler.
/// @return void
void circular_buf_free(cbuf buffer);

/// @brief 	Reset the buffer.
/// @param  buffer  A Circular Buffer handler.
/// @return void
void circular_buf_reset(cbuf buffer);

/// @brief 	Return the current size of the Circular Buffer.
/// @param  buffer  A Circular Buffer handler.
/// @return size_t 	The buffer size
size_t circular_buf_size(cbuf buffer);

/// @brief 	Return the current index of the Circular Buffer.
/// @param  buffer  A Circular Buffer handler.
/// @return size_t 	The index
size_t circular_buf_index(cbuf buffer);

/// @brief 	Return the current head of the Circular Buffer.
/// @param  buffer  A Circular Buffer handler.
/// @return size_t 	The index
size_t circular_buf_head(cbuf buffer);

/// @brief 	Return whether the Circular Buffer is empty or not.
/// @param  buffer  A Circular Buffer handler.
/// @return true 	When the buffer is empty.
/// @return false 	When the buffer is not empty.
bool circular_buf_is_empty(cbuf buffer);

/********************************************//**
 *  HELPER FUNCTIONS
 ***********************************************/
/// @brief 	Convert a String to an msg object (structure).
/// @param  char[]  The String to be converted.
/// @param 	*msg 	A pointer to an allocated msg struct where the convertion result will be saved.
/// @return void 	
void circular_buf_msg_structure(char str[], msg *msg);

/// @brief 	Convert an msg object to String (destructure).
/// @param 	*msg 	A pointer to an msg struct to be converted.
/// @param 	char[]	The String where the conversion result will be saved.
void circular_buf_msg_destructure(msg *msg, char str[]);

/// @brief 	Searche if a message (msg) exists in the Circular Buffer.
/// @param 	buffer 	A Circular Buffer handler.
/// @param 	*msg 	A pointer to an msg struct containing the message to be searched.
/// @return -1 		Error in reading from buffer
/// @return 1 		Message was foung in the Circular Buffer.
/// @return 0 		Message was not found in the Circular Buffer.
/// @note 	A more optimized search approach is not needed due to the number of the buffer elements.
int circular_buf_find(cbuf buffer, msg *msg);

/// @brief  Seed the Circular Buffer with BUFFER_SIZE/2 messages
/// Messages get random receivers from the nodes_list 
/// @param  buffer  A Circular Buffer handler.
/// @return void
void circular_buf_seed(cbuf buffer);

/********************************************//**
 *  DEBUG FUNCTIONS
 ***********************************************/
/// @brief 	Print the Circular Buffer status.
/// @param 	buffer 	A Circular Buffer handler.
/// @return void
void print_buffer_status(cbuf buffer);

/// @brief 	Print the Circular Buffer messages.
/// @param 	buffer 	A Circular Buffer handler.
/// @return void
void print_buffer(cbuf buffer);
