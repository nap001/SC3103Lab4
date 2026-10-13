//******************************************************************
// client_challenge.c   CE3103/SC3103 Exercise #4 Challenge Client
// Socket Client running on Ubuntu PC (or local machine)
// Forks up to 3 child processes using fork(), each sending
// a randomly generated number to the server concurrently,
// and displaying the result received from the server.
//
// Reference:
// - SC3103 Lab 4 Manual (Section 3 Challenge)
// - W9L3 Lecture Slides 46-47 (fork() process creation)
// - W9L3 Lecture Slides 23-25 (Socket Client IPC)
//
// Compilation: gcc -o client_challenge client_challenge.c
// Execution:   ./client_challenge <hostname/IP> <port>
//              (e.g., ./client_challenge 192.168.3.1 12345)
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

#define NUM_CHILDREN 3

void error(const char *msg) 
{ 
    perror(msg);
    exit(1);
}

// Function executed by each child process to connect to server and send a random number
void run_child_client(int child_index, const char *hostname, int port) 
{
    // Seed random generator uniquely per process using PID, timestamp, and index
    srand((unsigned int)(time(NULL) ^ (getpid() << 16) ^ (child_index * 1337)));

    // Random number between 1 and 100
    int rand_number = (rand() % 100) + 1;

    printf("[Child %d | PID %d] Started. Generated random number: %d\n", 
           child_index, getpid(), rand_number);

    // 1. Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        fprintf(stderr, "[Child %d | PID %d] Error opening socket\n", child_index, getpid());
        exit(1);
    }

    // 2. Resolve server host
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) 
    {
        fprintf(stderr, "[Child %d | PID %d] Error: no such host %s\n", child_index, getpid(), hostname);
        close(sockfd);
        exit(1);
    }

    // 3. Build server address
    struct sockaddr_in serv_addr;
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    // 4. Connect to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
        fprintf(stderr, "[Child %d | PID %d] Error connecting to server\n", child_index, getpid());
        close(sockfd);
        exit(1);
    }

    // 5. Send random number to server
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%d\n", rand_number);
    int n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
    {
        fprintf(stderr, "[Child %d | PID %d] Error writing to socket\n", child_index, getpid());
        close(sockfd);
        exit(1);
    }

    // 6. Read response from server
    memset(buffer, 0, sizeof(buffer));
    n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n < 0) 
    {
        fprintf(stderr, "[Child %d | PID %d] Error reading from socket\n", child_index, getpid());
        close(sockfd);
        exit(1);
    }

    // 7. Display server response
    // Strip trailing newline if present for cleaner output
    buffer[strcspn(buffer, "\r\n")] = '\0';
    printf(">>> [Child %d | PID %d] SUCCESS! Sent %d | Response: \"%s\"\n", 
           child_index, getpid(), rand_number, buffer);

    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[]) 
{ 
    if (argc < 3) 
    {
        fprintf(stderr, "Usage: %s <hostname/IP> <port>\n", argv[0]);
        fprintf(stderr, "Example for RPi:    %s 192.168.3.1 12345\n", argv[0]);
        fprintf(stderr, "Example local test:  %s localhost 12345\n", argv[0]);
        exit(1);
    }

    const char *hostname = argv[1];
    int port = atoi(argv[2]);
    pid_t child_pids[NUM_CHILDREN];

    printf("===============================================================\n");
    printf(" SC3103 Challenge Client (Parent PID: %d)\n", getpid());
    printf(" Forking %d concurrent child processes to test server\n", NUM_CHILDREN);
    printf(" Connecting to server at %s:%d\n", hostname, port);
    printf("===============================================================\n\n");

    // Fork up to three child processes
    for (int i = 0; i < NUM_CHILDREN; i++) 
    {
        pid_t pid = fork();

        if (pid < 0) 
        {
            perror("fork() failed");
            exit(1);
        } 
        else if (pid == 0) 
        {
            // In child process
            run_child_client(i + 1, hostname, port);
            // run_child_client terminates with exit(0)
        } 
        else 
        {
            // In parent process: store child PID
            child_pids[i] = pid;
        }
    }

    // Parent waits for all child processes to complete
    int status;
    for (int i = 0; i < NUM_CHILDREN; i++) 
    {
        pid_t completed_pid = waitpid(child_pids[i], &status, 0);
        if (completed_pid > 0) 
        {
            printf("[Parent] Child %d (PID %d) finished with exit status %d.\n", 
                   i + 1, completed_pid, WEXITSTATUS(status));
        }
    }

    printf("\n===============================================================\n");
    printf(" All %d child client processes completed successfully!\n", NUM_CHILDREN);
    printf("===============================================================\n");

    return 0;
}
