//******************************************************************
// client.c   CE3103/SC3103 Exercise #4 Section 3 Client
// Socket Client running on Ubuntu PC (or local machine)
// Connects to server on Raspberry Pi over network,
// prompts user to enter a number, sends it to the server,
// and displays the response received from the server.
//
// Reference: W9L3 Lecture Notes (Socket IPC)
// Compilation: gcc -o client client.c
// Execution:   ./client <hostname/IP> <port> (e.g., ./client 192.168.3.1 12345)
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) 
{ 
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) 
{ 
    int sockfd, port, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    if (argc < 3) 
    {
        fprintf(stderr, "Usage: %s <hostname/IP> <port>\n", argv[0]);
        fprintf(stderr, "Example for RPi:   %s 192.168.3.1 12345\n", argv[0]);
        fprintf(stderr, "Example local test: %s localhost 12345\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[2]);

    // 1. Create client socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    // 2. Resolve hostname / IP address
    server = gethostbyname(argv[1]);
    if (server == NULL) 
    {
        fprintf(stderr, "ERROR, no such host: %s\n", argv[1]);
        close(sockfd);
        exit(1);
    }

    // 3. Initialize server address structure
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    // 4. Connect to server
    printf("Connecting to server at %s:%d...\n", argv[1], port);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting to server");

    printf("Successfully connected to server.\n");

    // 5. Prompt user to enter a number
    printf("Please enter a number: ");
    memset(buffer, 0, sizeof(buffer));
    if (fgets(buffer, sizeof(buffer) - 1, stdin) == NULL) 
        error("ERROR reading input");

    // 6. Send number/message to server
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
        error("ERROR writing to socket");

    // 7. Receive reply from server
    memset(buffer, 0, sizeof(buffer));
    n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n < 0) 
        error("ERROR reading from socket");

    // 8. Display server message
    printf("Message received from server: %s", buffer);

    // 9. Close socket
    close(sockfd);
    return 0;
}
