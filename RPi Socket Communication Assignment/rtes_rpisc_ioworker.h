extern struct event_base *io_base;
void * io_worker_main(void *arg);
void io_handle_read(struct bufferevent *, void *);
void io_handle_write(struct bufferevent *, void *);