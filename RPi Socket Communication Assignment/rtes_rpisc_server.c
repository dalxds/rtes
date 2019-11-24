// LIBS load
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

// LIBEVENT load
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>

// FILES load
#include "rtes_rpisc_server.h"
#include "rtes_rpisc_ioworker.h"

// STRUCTS

struct event_base *server_base;

// *** FUNCTION - START *** //

int setnonblock(int fd) {
  int flags;
  flags = fcntl(fd, F_GETFL);

  if (flags < 0)
    return flags;

  flags |= O_NONBLOCK;

  if (fcntl(fd, F_SETFL, flags) < 0)
    return -1;

  return 0;
}

void on_accept(int fd, short ev, void *arg) {
  //here we have to implement the main funtionality of the server thread.
  int client_fd;
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
  struct event *on_accept_event;

  if (client_fd < 0) {
    warn("accept failed");
    return;
  }

  /* Set the client socket to non-blocking mode. */
  if (setnonblock(client_fd) < 0)
    warn("failed to set client socket non-blocking\n");

  printf("Accepted connection from %s\n", inet_ntoa(client_addr.sin_addr));
  /* TODO: add to io worker event base */
  on_accept_event = event_new(io_base, client_fd, EV_READ | EV_PERSIST, io_handle, NULL);

  if (event_add(on_accept_event, NULL) < 0)
    err(1, "failed to add event to the base");
}

// *** MAIN - START *** //
int server_main(int server_port, const char *server_ip) {
  int listen_fd;
  struct sockaddr_in listen_addr;
  int reuseaddr_on;
  struct event *server_event;
  /* Initialize libevent. */
  server_base = event_base_new();
  /* Create our listening socket. */
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (listen_fd < 0)
    err(1, "listen failed");

  //TODO: make server to run on server_ip arg
  memset(&listen_addr, 0, sizeof(listen_addr));
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_addr.s_addr = INADDR_ANY;
  listen_addr.sin_port = htons(server_port);

  if (bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0)
    err(1, "bind failed");

  if (listen(listen_fd, 5) < 0)
    err(1, "listen failed");

  reuseaddr_on = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on));

  /* Set the socket to non-blocking, this is essential in event
   * based programming with libevent. */
  if (setnonblock(listen_fd) < 0)
    err(1, "failed to set server socket to non-blocking");

  /* We now have a listening socket, we create a read event to
  * be notified when a client connects. */
  server_event = event_new(server_base, listen_fd, EV_READ | EV_PERSIST, on_accept, NULL);

  if (event_add(server_event, NULL) < 0)
    err(1, "failed to add event to the base");

  /* Start the event loop. */
  event_base_dispatch(server_base);
  return 0;
}