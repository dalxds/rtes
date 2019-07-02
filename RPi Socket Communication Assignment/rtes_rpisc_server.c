#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#define PORT 2288
#define TRUE 1
#define FALSE 0
#define MAX 1024

void message(int socket_fd) 
{ 
    char buff[MAX]; 
    int n; 
    char buff1[MAX]="From Server:"; 

    // infinite loop for chat 
    for (;;) { 
        bzero(buff, MAX); 
  
        // receive the message from client and copy it in buffer
        if (recv(socket_fd, &buff, sizeof(buff), 0) <0){
            puts("Recv failed");
            break;
        } 
        // print buffer which contains the client contents 
        printf("From client: %s\t ", buff); 
        //bzero(buff, MAX); 
        n = 0; 
        // copy server message in the buffer 
        while ((buff[n++] = getchar()) != '\n') ;
        strcat(buff1, buff); 
  
        // and send that buffer to client 
        send(socket_fd, buff1, sizeof(buff),0); 
  
        // if msg contains "Exit" then server exit and chat ended. 
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    } 
} 

int main () {

	char server_message[256];
    int end_server = FALSE;
    int new_sd = -1;
    int j,i, compress_array, close_conn;
    char buffer[1024] = {0}; 
    char msg_sent[2000] ={0};
	//create the server socket
	int socket_fd;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
    //Allow socket descriptor to be reuseable
    int on = 1;
    int rc = setsockopt(socket_fd, SOL_SOCKET,  SO_REUSEADDR,
                  (char *)&on, sizeof(on));
    if (rc < 0){
        perror("setsockopt() failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    /*Set socket to be nonblocking. All of the sockets for 
    the incoming connections will also be nonblocking since 
    they will inherit that state from the listening socket. */
    rc = ioctl(socket_fd, FIONBIO, (char *)&on);
    if (rc < 0){
        perror("ioctl() failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }


    //define the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; 
    server_address.sin_addr.s_addr = INADDR_ANY; 
    server_address.sin_port = htons( PORT ); 
    //memset(&(serverAddr.sin_zero), '\0', 8);

    // Forcefully attaching socket to the port 2288 
    rc = bind(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address));
    if (rc < 0) { 
        perror("bind failed"); 
        close(socket_fd);
        exit(EXIT_FAILURE); 
    } 

    //Set the listen back log   
    rc = listen(socket_fd, 32);
    if (rc < 0){ 
        perror("listen failed"); 
        close(socket_fd);
        exit(EXIT_FAILURE); 
    }

    //Initialize the pollfd structure
    struct pollfd fds[200];
    memset(fds, 0 , sizeof(fds));

    //Set up the initial listening socket
    fds[0].fd = socket_fd;
    fds[0].events = POLLIN;

    /* Initialize the timeout to 5 minutes. If no 
    activity after 5 minutes this program will end. 
    Timeout value is based on milliseconds */

    int timeout = (5 * 60 * 1000);
    int nfds = 1;

    /* Loop waiting for incoming connects or for incoming data
    on any of the connected sockets.*/
    do{
        //Call poll() and wait 5 minutes for it to complete. 
        printf("Waiting on poll()...\n");
        rc = poll(fds, nfds, timeout);

        //Check to see if the poll call failed. 
        if (rc < 0){
            perror("poll() failed");
            break;
        }

        //Check to see if the 5 minute time out expired
        if (rc == 0){
            printf("poll() timed out.End program.\n");
            break;
        }
        /*One or more descriptors are readable. Need to 
        determine which ones they are.*/

        int current_size = nfds;
        for (int i = 0; i < current_size; i++){
            /* Loop through to find the descriptors that returned
            POLLIN and determine whether it's the listening
            or the active connection. */

            //WHY ???
            if(fds[i].revents == 0){
                continue;
            }
            /* If revents is not POLLIN, it's an unexpected result
            log and end the server. */
            if(fds[i].revents != POLLIN){
                printf("Error! revents = %d\n", fds[i].revents);
                end_server = TRUE;
                break;
            }
            if (fds[i].fd == socket_fd){
                //Listening descriptor is readable.
                printf("Listening socket is readable\n");

                /* Accept all incoming connections that are
                queued up on the listening socket before we
                loop back and call poll again.*/
                do{
                    /* Accept each incoming connection. If
                    accept fails with EWOULDBLOCK, then we
                    have accepted all of them. Any other
                    failure on accept will cause us to end the
                    server.*/
                    new_sd = accept(socket_fd, NULL, NULL);
                    if (new_sd < 0){
                        if (errno != EWOULDBLOCK){
                            perror("accept() failed");
                            end_server = TRUE;
                        }
                    break;
                    }
                    //Add the new incoming connection to the pollfd structure
                    printf("New incoming connection - %d\n", new_sd);
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                    //send message
                    //message(socket_fd);
                    
                }
                //Loop back up and accept another incoming connection
                while (new_sd != -1);
            }
            /* This is not the listening socket, therefore an
            existing connection must be readable */
            else{
                printf("Descriptor %d is readable\n", fds[i].fd);
                close_conn = FALSE;
                int valread = recv(socket_fd , buffer, 1024,0); 
                printf("%s\n", buffer ); 
                printf("MESSAGE FROM CLIENT RECEIVED\n"); 

                char aem1[1000000] = "8764";
                char aem2[1000] = "8400";
                char msg_sent;
                strcat(aem1,"_");
                strcat(aem1,aem2);
                strcat(aem1,"_");

                time_t ltime;
                time(&ltime); 
                char a[10000];
                strcpy(a,ctime(&ltime));
    

                strcat(aem1,a);
                strcat(aem1,"_");
                strcat(aem1,buffer);

                //printf("message: %s", aem1);
    

                send(socket_fd , aem1 , strlen(aem1) , 0 ); 
                printf("MESSAGE TO CLIENT SENT\n"); 
                /* Receive all incoming data on this socket
                before we loop back and call poll again.*/
                do{
                    /* Receive data on this connection until the
                    recv fails with EWOULDBLOCK. If any other
                    failure occurs, we will close the
                    connection.*/
                    rc = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                    if (rc < 0){
                        if (errno != EWOULDBLOCK){
                            perror("recv() failed");
                            close_conn = TRUE;
                        }
                     break;
                    }
                    /* Check to see if the connection has been 
                    closed by the client. */
                    if (rc == 0){
                        printf("  Connection closed\n");
                        close_conn = TRUE;
                        break;
                    }
                    //data was received
                    int len = rc;
                    printf("  %d bytes received\n", len);
                    //Echo the data back to the client 
                    
                int valread = recv(socket_fd , buffer, 1024,0); 
                printf("%s\n", buffer ); 
                printf("MESSAGE FROM CLIENT RECEIVED\n"); 

                char aem1[1000000] = "8764";
                char aem2[1000] = "8400";
                 
                char msg_sent;
                strcat(aem1,"_");
                strcat(aem1,aem2);
                strcat(aem1,"_");

                time_t ltime;
                time(&ltime); 
                char a[10000];
                strcpy(a,ctime(&ltime));
    

                strcat(aem1,a);
                strcat(aem1,"_");
                strcat(aem1,buffer);
                int len1 = strlen(aem1);

                rc = send(fds[i].fd, aem1, len1, 0);
                    if (rc < 0){
                        perror("send() failed");
                        close_conn = TRUE;
                        break;
                    }
                } while(TRUE);
                /* If the close_conn flag was turned on, we need
                to clean up this active connection. This
                clean up process includes removing the
                descriptor.*/
                if (close_conn){
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    int compress_array = TRUE;
                }

            } // End of existing connection is readble
         } //End of loop through pollable descriptors 

        /* If the compress_array flag was turned on, we need
        to squeeze together the array and decrement the number
        of file descriptors. We do not need to move back the
        events and revents fields because the events will always
        be POLLIN in this case, and revents is output.*/

        if (compress_array){
            compress_array = FALSE;
            for (i = 0; i < nfds; i++){
                if (fds[i].fd == -1){
                    for(j = i; j < nfds; j++){
                        fds[j].fd = fds[j+1].fd;
                    }
                i--;
                nfds--;
                }
            }
        }

    } while (end_server == FALSE); // End of serving running.

    //Clean up all of the sockets that are open
    for (i = 0; i < nfds; i++){
        if(fds[i].fd >= 0){
            close(fds[i].fd);
        }
    }


return 0;

}



    /*int client_socket;
    if ((client_socket = accept(socket, NULL, NULL)) < 0) { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 

    //send the message
    send(client_socket, server_message, strlen(server_message), 0 ); 

    //close the socket
    close(socket);
	return 0;
} */
