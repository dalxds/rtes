extern struct event_base *io_base;
void * io_worker_main(void *arg);
void io_handle_read(struct bufferevent *bev, void *arg);
void io_handle_events(struct bufferevent *bev, short events, void *user_data);
