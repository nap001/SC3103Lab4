//******************************************************************
// client.c   SC3103 / CE3103 - Shared Circular Buffer Client
//
// Description:
// Multiprocess TCP Socket Client designed for Raspberry Pi (or PC).
// - Accepts command-line argument N.
// - Uses fork() to create N concurrent child processes.
// - Each child process:
//     1. Connects to the PC server via a TCP socket.
//     2. Generates and sends one integer to the server.
//     3. Receives the server's reply.
//     4. Prints: PID, value sent, buffer slot index, current buffer sum.
//     5. Closes the socket and exits.
// - Parent process waits for all N child processes to finish using wait().
//
// Compilation: gcc -Wall -Wextra -o client client.c
// Execution:   ./client <server_ip> <port> <N>  (e.g., ./client 192.168.1.100 12345 10)
//              ./client <N>                    (defaults to localhost:12345)
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 12345

void error(const char *msg) 
{
    perror(msg);
    exit(1);
}

// Child task executed by each forked child process
void run_child_client(int child_index, const char *hostname, int port) 
{
    // Unique random seed per child based on timestamp, PID, and child index
    srand((unsigned int)(time(NULL) ^ (getpid() << 16) ^ (child_index * 7919)));

    // Generate an integer to send (between 1 and 100)
    int value_to_send = (rand() % 100) + 1;

    // 1. Create client TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        fprintf(stderr, "[Child PID %d] Error opening socket\n", getpid());
        exit(1);
    }

    // 2. Resolve hostname/IP address
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) 
    {
        fprintf(stderr, "[Child PID %d] ERROR, no such host: %s\n", getpid(), hostname);
        close(sockfd);
        exit(1);
    }

    // 3. Set up server address structure
    struct sockaddr_in serv_addr;
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    // 4. Connect to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
        fprintf(stderr, "[Child PID %d] ERROR connecting to %s:%d\n", getpid(), hostname, port);
        close(sockfd);
        exit(1);
    }

    // 5. Send one integer to server
    char send_buf[64];
    snprintf(send_buf, sizeof(send_buf), "%d\n", value_to_send);
    int n = write(sockfd, send_buf, strlen(send_buf));
    if (n < 0) 
    {
        fprintf(stderr, "[Child PID %d] ERROR writing to socket\n", getpid());
        close(sockfd);
        exit(1);
    }

    // 6. Receive reply from server
    char recv_buf[128];
    memset(recv_buf, 0, sizeof(recv_buf));
    n = read(sockfd, recv_buf, sizeof(recv_buf) - 1);
    if (n < 0) 
    {
        fprintf(stderr, "[Child PID %d] ERROR reading from socket\n", getpid());
        close(sockfd);
        exit(1);
    }

    // 7. Parse reply: format is "value slot_index buffer_sum"
    int resp_val = 0;
    int slot_index = 0;
    int current_sum = 0;
    if (sscanf(recv_buf, "%d %d %d", &resp_val, &slot_index, &current_sum) < 3) 
    {
        fprintf(stderr, "[Child PID %d] ERROR parsing server response: %s\n", getpid(), recv_buf);
        close(sockfd);
        exit(1);
    }

    // 8. Print required output:
    // "PID, value sent, buffer slot index, current buffer sum"
    printf("[Child PID %d] Sent Value: %2d | Buffer Slot Index: %d | Current Buffer Sum: %d\n",
           getpid(), value_to_send, slot_index, current_sum);

    // 9. Close socket and exit child process
    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[]) 
{
    const char *hostname = DEFAULT_HOST;
    int port = DEFAULT_PORT;
    int n_processes = 0;

    // Command-line parsing:
    // Option 1: ./client <N>                     (uses default localhost:12345)
    // Option 2: ./client <server_ip> <port> <N>  (custom host and port)
    if (argc == 2) 
    {
        n_processes = atoi(argv[1]);
    } 
    else if (argc == 4) 
    {
        hostname = argv[1];
        port = atoi(argv[2]);
        n_processes = atoi(argv[3]);
    } 
    else 
    {
        fprintf(stderr, "===============================================================\n");
        fprintf(stderr, " Usage: %s <N>\n", argv[0]);
        fprintf(stderr, "        %s <server_ip> <port> <N>\n", argv[0]);
        fprintf(stderr, "---------------------------------------------------------------\n");
        fprintf(stderr, " Parameters:\n");
        fprintf(stderr, "   N           Number of child processes to fork (N > 0)\n");
        fprintf(stderr, "   server_ip   IP or hostname of PC Server (e.g. 192.168.1.100)\n");
        fprintf(stderr, "   port        Server TCP port (e.g. 12345)\n");
        fprintf(stderr, " Examples:\n");
        fprintf(stderr, "   Local Test:  %s 5\n", argv[0]);
        fprintf(stderr, "   RPi to PC:   %s 192.168.1.100 12345 10\n", argv[0]);
        fprintf(stderr, "===============================================================\n");
        exit(1);
    }

    if (n_processes <= 0) 
    {
        fprintf(stderr, "Error: N must be a positive integer greater than 0 (got %d).\n", n_processes);
        exit(1);
    }

    printf("===============================================================\n");
    printf(" SC3103 Shared Circular Buffer Client (RPi)\n");
    printf(" Target Server: %s:%d\n", hostname, port);
    printf(" Spawning N = %d child processes via fork()...\n", n_processes);
    printf("===============================================================\n\n");

    // Fork N child processes
    for (int i = 0; i < n_processes; i++) 
    {
        pid_t pid = fork();

        if (pid < 0) 
        {
            perror("fork() failed");
            exit(1);
        } 
        else if (pid == 0) 
        {
            // Inside Child Process
            run_child_client(i, hostname, port);
            // Child exits inside run_child_client()
        }
    }

    // Parent process waits for all N children to complete
    for (int i = 0; i < n_processes; i++) 
    {
        int status;
        wait(&status);
    }

    printf("\n===============================================================\n");
    printf(" [Parent PID %d] All %d child processes completed successfully.\n", getpid(), n_processes);
    printf("===============================================================\n");

    return 0;
}
