// LIBS load

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// LIBEVENT load

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

#include "rtes_rpisc_server.h"

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

// *** FUNCTIONS START *** //

void echo_read_cb(struct bufferevent *bev, void *ctx) {
  /* This callback is invoked when there is data to read on bev. */
  struct evbuffer *input = bufferevent_get_input(bev);
  struct evbuffer *output = bufferevent_get_output(bev);

  /* Copy all the data from the input buffer to the output buffer. */
  evbuffer_add_buffer(output, input);
}

void echo_event_cb(struct bufferevent *bev, short events, void *ctx) {
  if (events & BEV_EVENT_ERROR)
    perror("Error from bufferevent");
  if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
    bufferevent_free(bev);
  }
}

// Callback that runs on successful connection
void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx) {
  /* We got a new connection! Set up a bufferevent for it. */
  struct event_base *base = evconnlistener_get_base(listener);
  struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

  //set buffervent callbacks
  bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);

  bufferevent_enable(bev, EV_READ | EV_WRITE);
}

//Callback that will be invoked if an error occurs on the listener
void accept_error_cb(struct evconnlistener *listener, void *ctx) {
  struct event_base *base = evconnlistener_get_base(listener);
  int err = EVUTIL_SOCKET_ERROR();
  fprintf(stderr, "Got an error %d (%s) on the listener. "
          "Shutting down.\n", err, evutil_socket_error_to_string(err));
  event_base_loopexit(base, NULL);
}