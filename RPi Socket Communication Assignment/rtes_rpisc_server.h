void echo_read_cb(struct bufferevent *bev, void *ctx);

void echo_event_cb(struct bufferevent *bev, short events, void *ctx);

void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx);

void accept_error_cb(struct evconnlistener *listener, void *ctx);