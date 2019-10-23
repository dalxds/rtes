// LIBS load

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>

// LIBEVENT load

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

// FILES load
#include "rtes_rpisc_server.h"

// *** PROGRAM START *** //

int main(int argc, char **argv) {
  struct event_base *base;
  struct evconnlistener *listener;
  struct sockaddr_in sin;

  int port = 9876;

  if (argc > 1) {
    port = atoi(argv[1]);
  }
  if (port <= 0 || port > 65535) {
    puts("Invalid port");
    return 1;
  }

  base = event_base_new();
  if (!base) {
    puts("Couldn't open event base");
    return 1;
  }

  /* Clear the sockaddr before using it, in case there are extra
   * platform-specific fields that can mess us up. */
  memset(&sin, 0, sizeof(sin));
  /* This is an INET address */
  sin.sin_family = AF_INET;
  /* Listen on 0.0.0.0 */
  sin.sin_addr.s_addr = htonl(0);
  /* Listen on the given port. */
  sin.sin_port = htons(port);

  listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
                                     LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                     (struct sockaddr *)&sin, sizeof(sin));

  if (!listener) {
    perror("Couldn't create listener");
    return 1;
  }
  evconnlistener_set_error_cb(listener, accept_error_cb);

  event_base_dispatch(base);
  return 0;
}