//******************************************************************
// server_challenge.c   CE3103/SC3103 Exercise #4 Challenge Server
// Multithreaded Socket Server serving up to 3 clients simultaneously
// using a Producer-Consumer based Semaphore scheme.
//
// Reference:
// - SC3103 Lab 4 Manual (Section 3 Challenge)
// - W8L2 Lecture Slides 45-52 (Producer-Consumer & Semaphores)
// - W9L3 Lecture Slides 26-29 (Socket Server IPC)
//
// Compilation: gcc -o server_challenge server_challenge.c -lpthread
// Execution:   ./server_challenge <port> (e.g., ./server_challenge 12345)
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define MAX_WORKERS 3   // Up to 3 threads to serve up to 3 clients simultaneously
#define BUFF_SIZE   3   // Bounded circular buffer capacity

// Shared data structure for Producer-Consumer bounded buffer
typedef struct {
    int buf[BUFF_SIZE];         // Circular queue storing client socket descriptors
    int in;                     // Index for producer (main thread) to put connection
    int out;                    // Index for consumer (worker thread) to get connection
    sem_t buffer;               // Counting semaphore for empty buffer slots (initialized to BUFF_SIZE)
    sem_t data;                 // Counting semaphore for available data/connections (initialized to 0)
    pthread_mutex_t mutex;      // Mutex to protect circular buffer index updates
} SharedQueue;

static SharedQueue shared;
static int server_sockfd = -1;

void error(const char *msg) 
{ 
    perror(msg);
    exit(1);
}

// Graceful cleanup on SIGINT (Ctrl+C)
void handle_sigint(int sig) 
{ 
    (void)sig;
    printf("\n[Server] Shutting down server and cleaning up resources...\n");
    if (server_sockfd >= 0) 
    {
        close(server_sockfd);
    }
    sem_destroy(&shared.data);
    sem_destroy(&shared.buffer);
    pthread_mutex_destroy(&shared.mutex);
    exit(0);
}

// Consumer Thread Function (Worker)
void *worker_consumer(void *arg) 
{ 
    int worker_id = *(int *)arg;
    free(arg);

    printf("[Worker %d] Initialized and waiting for client connections.\n", worker_id);

    while (1) 
    { 
        // 1. Wait for available client connection in the shared queue (decrement sem_data)
        sem_wait(&shared.data);

        // 2. Lock mutex to safely retrieve connection descriptor from circular queue
        pthread_mutex_lock(&shared.mutex);
        int newsockfd = shared.buf[shared.out];
        shared.out = (shared.out + 1) % BUFF_SIZE;
        pthread_mutex_unlock(&shared.mutex);

        // 3. Signal that an empty slot is now available in buffer (increment sem_buffer)
        sem_post(&shared.buffer);

        printf("\n>>> [Worker %d] Serving client on socket descriptor %d...\n", worker_id, newsockfd);

        // 4. Read client request
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        int n = read(newsockfd, buffer, sizeof(buffer) - 1);
        if (n < 0) 
        {
            perror("[Worker] ERROR reading from socket");
            close(newsockfd);
            continue;
        }

        // 5. Parse number and multiply by 5
        int number = atoi(buffer);
        int result = number * 5;

        // Artificial slight sleep (e.g., 500ms) to demonstrate concurrent multi-client handling
        usleep(500000); 

        // 6. Format and send response back to client
        char response[256];
        snprintf(response, sizeof(response), 
                 "Worker #%d processed: %d * 5 = %d\n", 
                 worker_id, number, result);

        n = write(newsockfd, response, strlen(response));
        if (n < 0) 
        {
            perror("[Worker] ERROR writing to socket");
        } 
        else 
        {
            printf("<<< [Worker %d] Done serving socket %d: Input %d -> Output %d\n", 
                   worker_id, newsockfd, number, result);
        }

        // 7. Close client socket and return to waiting
        close(newsockfd);
    } 

    return NULL;
}

int main(int argc, char *argv[]) 
{ 
    int port;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t workers[MAX_WORKERS];

    if (argc < 2) 
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        fprintf(stderr, "Example: %s 12345\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);
    signal(SIGINT, handle_sigint);

    // 1. Initialize Producer-Consumer Shared Queue & Semaphores
    shared.in = 0;
    shared.out = 0;
    sem_init(&shared.data, 0, 0);               // Initially 0 items in buffer
    sem_init(&shared.buffer, 0, BUFF_SIZE);     // Initially BUFF_SIZE empty slots
    pthread_mutex_init(&shared.mutex, NULL);

    // 2. Spawn up to three worker threads (Consumers) to serve up to three clients simultaneously
    for (int i = 0; i < MAX_WORKERS; i++) 
    {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        if (pthread_create(&workers[i], NULL, worker_consumer, id) != 0) 
        {
            error("Failed to spawn worker thread");
        }
    }

    // 3. Create server socket
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) 
        error("ERROR opening socket");

    int opt = 1;
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 4. Bind server address
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(server_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    // 5. Listen with backlog
    listen(server_sockfd, 10);
    printf("===============================================================\n");
    printf(" SC3103 Challenge Server started on port %d\n", port);
    printf(" Semaphore-managed Thread Pool: %d concurrent worker threads\n", MAX_WORKERS);
    printf(" Ready to serve up to %d clients simultaneously!\n", MAX_WORKERS);
    printf("===============================================================\n");

    clilen = sizeof(cli_addr);

    // 6. Main loop acts as Producer: accepts incoming connections and enqueues them
    while (1) 
    {
        int newsockfd = accept(server_sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
        {
            perror("ERROR on accept");
            continue;
        }

        printf("\n[Producer] New connection accepted (fd: %d). Enqueuing...\n", newsockfd);

        // Wait until an empty slot is available in the bounded buffer
        sem_wait(&shared.buffer);

        // Put connection in circular queue
        pthread_mutex_lock(&shared.mutex);
        shared.buf[shared.in] = newsockfd;
        shared.in = (shared.in + 1) % BUFF_SIZE;
        pthread_mutex_unlock(&shared.mutex);

        // Signal waiting consumer worker thread that new connection data is ready
        sem_post(&shared.data);
    }

    // Clean up
    close(server_sockfd);
    sem_destroy(&shared.data);
    sem_destroy(&shared.buffer);
    pthread_mutex_destroy(&shared.mutex);

    return 0;
}
