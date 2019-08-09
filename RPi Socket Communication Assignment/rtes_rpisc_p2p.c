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
#include <stdbool.h>
#include <pthread.h>
#include <poll.h>

//my files
#include "rtes_rpisc_ringbuffer.h"

/**************************************************
*                       INIT                      *
***************************************************/

#define PORT_DEFAULT          2288
#define AEM_LIST_LENGTH       50
#define MAX_WAITING_CLIENTS   50

/* LIBEVENT FLAGS */
#define EVLOOP_NO_EXIT_ON_EMPTY 0x04

#define EV_TIMEOUT      0x01
#define EV_READ         0x02
#define EV_WRITE        0x04
#define EV_SIGNAL       0x08
#define EV_PERSIST      0x10
#define EV_ET           0x20

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
  uint64_t  msgcounter;
} p2p_node_t;

typedef struct {
  uint32_t      aem_sender;
  uint32_t      aem_receiver;
  uint64_t      timestamp;
  char          msg_body[256];
} p2p_message_t;

/**************************************************
*                    FUNCTIONS                    *
***************************************************/

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
  return client_sock;
}

/* IO THREAD FUNCTIONS */
void cb_func(evutil_socket_t fd, short what, void *arg) {
  const char *data = arg;
  printf("Got an event on socket %d:%s%s%s%s [%s]",
         (int) fd,
         (what & EV_TIMEOUT) ? " timeout" : "",
         (what & EV_READ)    ? " read" : "",
         (what & EV_WRITE)   ? " write" : "",
         (what & EV_SIGNAL)  ? " signal" : "",
         data);
  if(what & EV_READ) {
    //TODO: read from node and send to DATA WORKER
  }
  if(what & EV_WRITE){
    //TODO: ask from DATA WORKER to send the next message for this node
  }
}

/**************************************************
*                THREAD FUNCTIONS                 *
***************************************************/

// IO THREAD
// takes sockets
// does the epoll wait
// sends and receives data from data handler thread
void *io_worker_main(void *args) {

  //initialize event base
  struct event_base *base = event_base_new();


  // run event loop (event polling)
  int event_base_loop(struct event_base * base, EVLOOP_NO_EXIT_ON_EMPTY);



}

// DATA HANDLER THREAD
// controls the buffer
// sends data to IO thread
// receives data from IO thread and adds them to the buffer
// produces the random message
void *data_handler_main(void *args) {
  //initialize buffer
  circular_buf_t buffer;
  cbuf_handle_t circular_buf_init(uint8_t *buffer, size_t size);

  //TODO: tracks the items counter send for each node respectively (along with statust in AEM_list ?)

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

  /* POLL SETUP */
  struct pollfd fds[AEM_LIST_LENGTH];
  memset(fds, 0, sizeof(fds));

  /* IO WORKER */
  pthread_t io_worker;

  //create IO WORKER Thread
  //TODO: encapsulate passing data (?)
  if (pthread_create(&io_worker, NULL, io_worker_main, epfd) < 0) {
    printf("An error occured: %d", err);
    return -1;
  }

  //FOR SERVER AND CLIENT THREADS
  //call function to create event
  struct event *event_new(
      struct event_base * base,
      evutil_socket_t fd,
      short what,
      event_callback_fn cb,
      void *arg);
  //add event to event loop with no timeout
  int event_add(struct event *ev, NULL);


}