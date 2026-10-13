//******************************************************************
// server.c   CE3103/SC3103 Exercise #4 Section 3 Server
// Multithreaded Socket Server running on Raspberry Pi
// Spawns a thread to process incoming client request,
// multiplies the received number by 5, and returns the result.
//
// Reference: W9L3 Lecture Notes (Socket IPC) & W8L2 (Multithreading)
// Compilation: gcc -o server server.c -lpthread
// Execution:   ./server <port> (e.g., ./server 12345)
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) 
{ 
    perror(msg);
    exit(1);
}

/* Thread function to process an individual client connection */
void *process_client(void *socket_desc) 
{ 
    int newsockfd = *(int *)socket_desc;
    free(socket_desc); // Free heap allocation for socket descriptor

    char buffer[256];
    memset(buffer, 0, sizeof(buffer));

    // Read message/number sent from client
    int n = read(newsockfd, buffer, sizeof(buffer) - 1);
    if (n < 0) 
    {
        perror("[Server Thread] ERROR reading from socket");
        close(newsockfd);
        pthread_exit(NULL);
    }

    printf("[Server Thread %lu] Received number/message: %s", (unsigned long)pthread_self(), buffer);

    // Convert string to integer and multiply by 5 as required
    int number = atoi(buffer);
    int result = number * 5;

    // Prepare response message
    char response[256];
    snprintf(response, sizeof(response), "Server result: %d (original %d * 5)\n", result, number);

    // Send the calculated result back to the client
    n = write(newsockfd, response, strlen(response));
    if (n < 0) 
    {
        perror("[Server Thread] ERROR writing to socket");
    } 
    else 
    {
        printf("[Server Thread %lu] Replied: %s", (unsigned long)pthread_self(), response);
    }

    close(newsockfd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) 
{ 
    int sockfd, port;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) 
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        fprintf(stderr, "Example: %s 12345\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);

    // 1. Create stream socket (IPv4 TCP)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    // Allow immediate reuse of the local address/port
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Prepare sockaddr_in structure
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // 3. Bind socket to the address and port
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    // 4. Listen for incoming client connections
    listen(sockfd, 5);
    printf("===========================================\n");
    printf(" SC3103 Server started on port %d\n", port);
    printf(" Waiting for incoming client connections...\n");
    printf("===========================================\n");

    clilen = sizeof(cli_addr);

    // 5. Accept and handle incoming connections
    while (1) 
    {
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
        {
            perror("ERROR on accept");
            continue;
        }

        printf("\n[Main Thread] Accepted new connection (fd: %d).\n", newsockfd);

        // Dynamically allocate memory for socket descriptor to prevent race condition
        int *client_sock = malloc(sizeof(int));
        if (client_sock == NULL) 
        {
            perror("malloc failed");
            close(newsockfd);
            continue;
        }
        *client_sock = newsockfd;

        // Spawn a thread to serve client
        pthread_t worker_thread;
        if (pthread_create(&worker_thread, NULL, process_client, (void *)client_sock) != 0) 
        {
            perror("ERROR creating client thread");
            close(newsockfd);
            free(client_sock);
        } 
        else 
        {
            // Detach thread to automatically release thread resources upon exit
            pthread_detach(worker_thread);
        }
    }

    close(sockfd);
    return 0;
}
