/// @brief Server Thread Function
/// Binds and Listens on the the specified PORT
/// waiting for incoming connections
void *server_main(void *arg);

/// @ brief Callback Function triggered when server accepts a new connection
void on_accept(int fd, short ev, void *arg);