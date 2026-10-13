//******************************************************************
// server.c   SC3103 / CE3103 - Shared Circular Buffer Server
//
// Description:
// Multithreaded TCP Socket Server running on PC.
// - Listens for incoming client TCP connections.
// - Spawns a dedicated POSIX thread (pthread) per client connection.
// - Maintains a shared circular buffer of size 5, write index, and sum.
// - Enters a critical section protected by a mutex (or semaphore) to:
//     1. Insert the received integer into the circular buffer.
//     2. Advance the write index: (write_index + 1) % 5.
//     3. Update the buffer sum.
//     4. Print: received value, slot index, current buffer sum, and buffer state.
// - Replies back to client with: value, slot index, current buffer sum.
//
// Compilation: gcc -Wall -Wextra -o server server.c -lpthread
// Execution:   ./server <port>           (e.g., ./server 12345)
//              ./server                  (defaults to port 12345)
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 5
#define DEFAULT_PORT 12345

// Set to 1 to use sem_t binary semaphore instead of pthread_mutex_t
#ifndef USE_SEMAPHORE
#define USE_SEMAPHORE 0
#endif

// Shared circular buffer state structure
typedef struct {
    int buffer[BUFFER_SIZE]; // Circular buffer slots (size 5)
    int write_index;         // Next write slot index (0 to 4)
    int sum;                 // Current sum of all elements in the buffer
#if USE_SEMAPHORE
    sem_t sem;               // Binary semaphore protecting critical section
#else
    pthread_mutex_t mutex;   // Mutex protecting critical section
#endif
} SharedBufferState;

// Global shared state
static SharedBufferState shared;
static int server_sockfd = -1;

void error(const char *msg) 
{
    perror(msg);
    exit(1);
}

// Graceful cleanup on SIGINT (Ctrl + C)
void handle_sigint(int sig) 
{
    (void)sig;
    printf("\n[Server] Interrupted by signal. Shutting down...\n");
    if (server_sockfd >= 0) 
    {
        close(server_sockfd);
    }
#if USE_SEMAPHORE
    sem_destroy(&shared.sem);
#else
    pthread_mutex_destroy(&shared.mutex);
#endif
    printf("[Server] Clean shutdown complete.\n");
    exit(0);
}

// Thread routine to process an individual client connection
void *client_handler(void *socket_desc) 
{
    int client_fd = *(int *)socket_desc;
    free(socket_desc); // Free memory allocated on heap for socket descriptor

    char read_buf[256];
    memset(read_buf, 0, sizeof(read_buf));

    // 1. Receive integer sent by client
    int n = read(client_fd, read_buf, sizeof(read_buf) - 1);
    if (n <= 0) 
    {
        perror("[Server Thread] ERROR reading from client socket");
        close(client_fd);
        pthread_exit(NULL);
    }

    // Parse the received integer
    int value = atoi(read_buf);

    // Variables to store snapshot of state computed within critical section
    int inserted_slot = 0;
    int updated_sum = 0;

    // ------------------- CRITICAL SECTION START -------------------
#if USE_SEMAPHORE
    sem_wait(&shared.sem);
#else
    pthread_mutex_lock(&shared.mutex);
#endif

    // Determine target slot in circular buffer
    inserted_slot = shared.write_index;

    // Overwrite the slot and update the cumulative sum:
    // Subtract old value currently residing in this slot, add new value
    int old_val = shared.buffer[inserted_slot];
    shared.buffer[inserted_slot] = value;
    shared.sum = shared.sum - old_val + value;
    updated_sum = shared.sum;

    // Advance write index in circular fashion
    shared.write_index = (shared.write_index + 1) % BUFFER_SIZE;

    // Print details on the server as required:
    // "value, slot index, buffer sum"
    printf("[Server Thread %lu] Received: %d | Slot Index: %d | Buffer Sum: %d | State: [%d, %d, %d, %d, %d] | Next Slot: %d\n",
           (unsigned long)pthread_self(),
           value,
           inserted_slot,
           updated_sum,
           shared.buffer[0], shared.buffer[1], shared.buffer[2], shared.buffer[3], shared.buffer[4],
           shared.write_index);

#if USE_SEMAPHORE
    sem_post(&shared.sem);
#else
    pthread_mutex_unlock(&shared.mutex);
#endif
    // -------------------- CRITICAL SECTION END --------------------

    // 2. Prepare reply to client: "value slot_index buffer_sum\n"
    char response[256];
    snprintf(response, sizeof(response), "%d %d %d\n", value, inserted_slot, updated_sum);

    n = write(client_fd, response, strlen(response));
    if (n < 0) 
    {
        perror("[Server Thread] ERROR writing response to socket");
    }

    // Close client connection and exit thread
    close(client_fd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) 
{
    int port = DEFAULT_PORT;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc >= 2) 
    {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) 
        {
            fprintf(stderr, "Error: Invalid port number '%s'. Must be 1-65535.\n", argv[1]);
            exit(1);
        }
    }

    // Set up signal handler for clean shutdown
    signal(SIGINT, handle_sigint);

    // Initialize shared circular buffer and synchronization primitive
    memset(&shared, 0, sizeof(shared));
    shared.write_index = 0;
    shared.sum = 0;

#if USE_SEMAPHORE
    if (sem_init(&shared.sem, 0, 1) != 0) 
    {
        error("ERROR initializing binary semaphore");
    }
#else
    if (pthread_mutex_init(&shared.mutex, NULL) != 0) 
    {
        error("ERROR initializing mutex");
    }
#endif

    // 1. Create TCP stream socket
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) 
        error("ERROR opening server socket");

    // Allow reuse of local address/port immediately after restarting server
    int opt = 1;
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Configure server address structure
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // 3. Bind socket to specified port
    if (bind(server_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on socket bind");

    // 4. Listen for incoming connections
    if (listen(server_sockfd, 20) < 0) 
        error("ERROR on socket listen");

    printf("===============================================================\n");
    printf(" SC3103 Shared Circular Buffer Server (PC)\n");
    printf(" Listening on Port: %d\n", port);
    printf(" Buffer Size:       %d slots (circular overwrite)\n", BUFFER_SIZE);
#if USE_SEMAPHORE
    printf(" Synchronization:   POSIX Semaphore (sem_t)\n");
#else
    printf(" Synchronization:   POSIX Mutex (pthread_mutex_t)\n");
#endif
    printf(" Waiting for RPi client connections... (Press Ctrl+C to stop)\n");
    printf("===============================================================\n");

    clilen = sizeof(cli_addr);

    // 5. Connection acceptance loop: spawn one thread per client
    while (1) 
    {
        int newsockfd = accept(server_sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) 
        {
            perror("[Server] ERROR on accept");
            continue;
        }

        // Dynamically allocate memory for socket descriptor to prevent race conditions
        int *client_sock = malloc(sizeof(int));
        if (client_sock == NULL) 
        {
            perror("[Server] malloc failed for client socket descriptor");
            close(newsockfd);
            continue;
        }
        *client_sock = newsockfd;

        // Create a new thread to serve the client
        pthread_t worker_tid;
        if (pthread_create(&worker_tid, NULL, client_handler, (void *)client_sock) != 0) 
        {
            perror("[Server] ERROR creating client thread");
            close(newsockfd);
            free(client_sock);
        } 
        else 
        {
            // Detach thread to automatically reclaim resources when it exits
            pthread_detach(worker_tid);
        }
    }

    close(server_sockfd);
    return 0;
}
