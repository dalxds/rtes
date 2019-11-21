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

// CONSTANTS
const int   S_PORT    =   2288;
const char  S_IP[16]  =   "127.0.0.1";

// *** PROGRAM START *** //

int main(int argc, char **argv) {
  server_main(S_PORT, S_IP);
  return 0;
}