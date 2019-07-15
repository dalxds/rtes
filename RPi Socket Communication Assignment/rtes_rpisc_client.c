#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#define PORT 2288
#define MAX 1024

// void message(int socket_fd) 
// { 
//     char buff[MAX], msg[MAX-20]; 
//     int n; 
//     for (;;) { 
//         bzero(buff, sizeof(buff)); 
//         printf("Enter the string : "); 
//         n = 0; 
//         while ((buff[n++] = getchar()) != '\n'); 
//         //send message to server
//         if (send(socket_fd, buff, sizeof(buff), 0) < 0){
//             printf("Send failed \n");
//             break;
//         }
//         //bzero(buff, sizeof(buff)); 
//         //recieve message from server
//         if (recv(socket_fd, &buff, sizeof(buff), 0) <0){
//             puts("Recv failed");
//             break;
//         } 
//         printf("From Server : %s", buff); 
//         if ((strncmp(buff, "exit", 4)) == 0) { 
//             printf("Client Exit...\n"); 
//             break; 
//         } 
//     } 
// } 

int main() {

    //nickname for every client
    char name[20];
    printf("Please enter your nick "); 
    scanf("%s",name);
    
	//create a socket
	int socket_fd;
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
        printf("\nCreating a socket Failed \n");
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }

    //specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    socklen_t len = sizeof(server_address);
    //memset(&(serverAddr.sin_zero), '\0', 8);


    //check for error with the connection
    if (connect(socket_fd, (struct sockaddr *) &server_address, len) < 0) { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    else{
        printf("Connected to the server..\n"); 
    }

    //send message
    //message(socket_fd);
    char buff[MAX], msg[MAX-20], get_msg[MAX]; 
    
    while(1)
    {
        //Get message
        printf(">>>"); 
        scanf("%s",msg);
        if(strcmp(msg,"exit") == 0){
            printf("Exit Client...\n");
            break;
        }
        //Edit message
        strcpy(buff, name);
        strcat(buff, " : ");
        strcat(buff, msg);
        //Send message
        int byte = send(socket_fd, buff, strlen(buff), 0);
        //Error Handling
        if(byte == -1){
            perror("Error on Send");
            break;
        }
        if(byte == 0){
            printf("Connection've been closed");;
            break;
        }
        //Get reply to other users
        recv(socket_fd, &get_msg, MAX-1, 0);
        printf("A: %s", get_msg);
    }

    // receive data from the server
    //char server_response[256];
    //recv(socket, &server_response, sizeof(server_response), 0);
    //print out the server's response
    //printf("The server sent data: %s\n", server_response);

    //close socket
    close(socket_fd);

	return 0;
}

