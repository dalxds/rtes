// STRUCTS
extern struct event_base *io_base;

/// @brief IO Worker Thread Function
void * io_worker_main(void *arg);

/// @brief IO Read Callback
void io_handle_read(struct bufferevent *bev, void *arg);

/// @brief IO Events Callback
void io_handle_events(struct bufferevent *bev, short events, void *user_data);

/// @brief Generates random message when called and puts it in dw_buffer
void io_generate_random_message(int fd, short events, void *arg);
