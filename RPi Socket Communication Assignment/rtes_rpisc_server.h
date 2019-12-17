// main server function
void *server_main(void *arg);

//sets socket to non-blocking mode
//=> returns 0 on success, -1 on error
int setnonblock(int fd);

/*  callback function on accept
 *  will propably be a function of the IO Worker Thread
 */ 
void on_accept(int fd, short ev, void *arg);