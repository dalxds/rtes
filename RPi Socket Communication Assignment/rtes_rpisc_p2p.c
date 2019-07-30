#include <stdint.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>

/**************************************************
*                       INIT                      *
***************************************************/

#define PORT_DEFAULT       2288
#define AEM_LIST_LENGTH    50

/**************************************************
*                     STRUCTS                     *
***************************************************/

/* MAIN APP STRUCTS */
typedef struct {
  /* Client socket identifier */
  int client_sock;
  /* Address bound to the client socket interface
   * this is NOT the peer address */
  struct sockaddr_in local_addr;
  /* Server socket identifier */
  int server_sock;
  /* Address bound to the server socket */
  struct sockaddr_in server_addr;
  /* */
  void *user_data;
} p2p_struct_t;

typedef struct {
  //TODO: change to proper data types
  uint32_t  id;
  uint32_t  ip;
  bool      status;
  uint64_t  msgcounter
} p2p_node_t;

typedef struct {
  uint32_t      aem_sender;
  uint32_t      aem_receiver;
  uint64_t      timestamp;
  char[256]     msg_body;
} p2p_message_t;

/* BUFFER STRUCTS */
struct circular_buf_t {
  uint8_t * buffer;
  size_t head;
  size_t tail;
  size_t max; //of the buffer
  bool full;
  size_t counter; // TODO: check if it's correct type
};

// circular buffer structure
typedef struct circular_buf_t circular_buf_t;
// Handle type, the way users interact with the API
typedef circular_buf_t *cbuf_handle_t;

/**************************************************
*                    FUNCTIONS                    *
***************************************************/

/* BUFFER FUNCTIONS */

// private helpers
static void advance_pointer(cbuf_handle_t cbuf) {
  assert(cbuf);

  if(cbuf->full) {
    cbuf->tail = (cbuf->tail + 1) % cbuf->max;
  }

  cbuf->head = (cbuf->head + 1) % cbuf->max;

  // We mark full because we will advance tail on the next time around
  cbuf->full = (cbuf->head == cbuf->tail);
}

static void retreat_pointer(cbuf_handle_t cbuf) {
  assert(cbuf);

  cbuf->full = false;
  cbuf->tail = (cbuf->tail + 1) % cbuf->max;
}

/// Pass in a storage buffer and size, returns a circular buffer handle
/// Requires: buffer is not NULL, size > 0
/// Ensures: cbuf has been created and is returned in an empty state
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

/// Adds data to buffer
/// Old data is overwritten
/// Requires: cbuf is valid and created by circular_buf_init
void circular_buf_put(cbuf_handle_t cbuf, uint8_t data) {
  assert(cbuf && cbuf->buffer);

  cbuf->buffer[cbuf->head] = data;

  advance_pointer(cbuf);
}

/// Retrieve a value from the buffer
/// Requires: cbuf is valid and created by circular_buf_init
/// Returns 0 on success, -1 if the buffer is empty
int circular_buf_get(cbuf_handle_t cbuf, uint8_t *data) {
  assert(cbuf && data && cbuf->buffer);

  int r = -1;

  if(!circular_buf_empty(cbuf)) {
    *data = cbuf->buffer[cbuf->tail];
    retreat_pointer(cbuf);

    r = 0;
  }

  return r;
}

/* MAIN APP FUNCTIONS */

void p2p_init(p2p_struct_t *p2p) {
  memset(p2p, 0, sizeof(p2p));
}

void p2p_close(p2p_struct_t *p2p) {
  close(p2p->client_sock);
  close(p2p->server_sock);
}

//p2p_listen(): p2p struct, host IP and port => server socket
// -> the server part 1
//socket(), bind(), listen()
int p2p_listen(p2p_struct_t *p2p, char *host, char *port) {
  int call_result; // returns the result of each call
  int server_sock; // server socket descriptor
  struct sockaddr_in server_addr;
  int port_as_int;

  port_as_int = atoi(port);
  if(port_as_int <= 0) {
    port_as_int = PORT_DEFAULT;
  }

  /* Create the socket, see p2p_join for an explanation of the parameters. */
  if((server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    return server_sock;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_as_int);
  server_addr.sin_addr.s_addr = inet_addr(host);
  if(server_addr.sin_addr.s_addr == INADDR_NONE) {
    server_addr.sin_addr.s_addr = INADDR_ANY;
  }

  if((call_result = bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr))) != 0) {
    close(server_sock);
    return call_result;
  }

  if((call_result = listen(server_sock, MAX_WAITING_CLIENTS)) < 0) {
    close(server_sock);
    return call_result;
  }

  p2p->server_sock = server_sock;
  p2p->server_addr = server_addr;
  return server_sock;
}

//p2p_accept(): p2p struct => peer socket
// -> the server part 2
//accept()
int p2p_accept(p2p_struct_t *p2p) {
  int peer_socket;
  struct sockaddr_in peer_addr;
  socklen_t addr_ln;

  /* accept a peer */
  if((peer_socket = accept(p2p->server_sock, (struct sockaddr *) &peer_addr, &addr_ln)) < 0) {
    close(peer_socket);
  }

  return peer_socket;
}

//p2p_join(): p2p struct, host IP and port => client socket
// -> the client part
//connect()
int p2p_join(p2p_struct_t *p2p, char *host, char *port) {
  struct sockaddr_in h_addr;
  struct sockaddr_in l_addr;
  int client_sock;
  int call_state;
  socklen_t l_add_len;

  memset(&h_addr, 0, sizeof(h_addr));
  /* We connect using TCP/Ipv4 */
  h_addr.sin_family = AF_INET;
  /* Parse the IP and format it for the low-level struct */
  h_addr.sin_addr.s_addr = inet_addr(host);
  /* Parse the port string in integer (atoi) and convert to network byte order
   * (htons) */
  h_addr.sin_port = htons(atoi(port));

  if(h_addr.sin_addr.s_addr == INADDR_NONE) {
    return -1;
  }

  /* Create the socket :
   *  - AF_INET (equivalent to PF_INET) : IPv4
   *  - SOCK_STREAM : a TCP STREAM
   *  - IPPROTO_TCP : tells that the protocol is TCP (even if SOCK_STREAM
   *    actualy implies it).
   **/
  if((client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    /* Failed : return socket error code */
    return client_sock;
  }

  if((call_state = connect(client_sock, (struct sockaddr *) &h_addr, sizeof(h_addr))) < 0) {
    /* Failed : return connect error code */
    close(client_sock);
    return call_state;
  }

  /* get the IP of my bound interface */
  if((call_state = getsockname(client_sock, (struct sockaddr *) &l_addr, &l_add_len)) < 0) {
    close(client_sock);
    return -1;
  }

  /** send msg
  *  if(send(client_sock, &msg, sizeof(msg), 0) != sizeof(msg)) {
  *   close(client_sock);
  *   return -1;
  * }
  */

  /* populate the p2p struct */
  p2p->local_addr = l_addr;
  p2p->client_sock = client_sock;
  return client_sock
}

/**************************************************
*                      MAIN                       *
***************************************************/

int main(int argc, char *argv[]) {
  int listen = 0;
  char *arg_ip;
  char *arg_port;
  p2p_struct_t p2p;

  p2p_init(&p2p);

  //epoll file descriptor
  epfd = epoll_create(AEM_LIST_LENGTH);

}