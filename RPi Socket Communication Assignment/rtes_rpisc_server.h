// main server function
int server_main(int server_port, const char *server_ip);

//sets socket to non-blocking mode
//=> returns 0 on success, -1 on error
int setnonblock(int fd);

/*	callback function on accept
 *	will propably be a function of the IO Worker Thread
 */	
void on_accept(int fd, short ev, void *arg);