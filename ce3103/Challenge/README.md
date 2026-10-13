# Challenge: Multithreaded Server with Semaphore Producer-Consumer & Multi-Process Client

## Overview
This challenge extends the basic client-server model to demonstrate high-concurrency embedded systems concepts:
1. **Server (`server_challenge.c`):**
   - Runs on the Raspberry Pi.
   - Spawns up to **three worker threads** to serve up to **three clients simultaneously**.
   - Implements a **Producer-Consumer scheme** using counting semaphores (`sem_t`) and a mutex (`pthread_mutex_t`) to bound and regulate incoming connections:
     - `sem_buffer`: Initialized to 3 (buffer capacity). Tracks empty connection slots.
     - `sem_data`: Initialized to 0. Signals waiting worker threads when an incoming client socket has been enqueued.
     - `mutex`: Protects the circular queue index updates (`in` and `out`).
     - Main thread acts as the **Producer**: accepts connections and places them into the shared queue.
     - 3 Worker threads act as the **Consumers**: pick up client connections from the shared queue, multiply the number by 5, and transmit the reply.
2. **Client (`client_challenge.c`):**
   - Runs on the Ubuntu computer.
   - Uses `fork()` to create **three concurrent child processes**.
   - Each child independently:
     - Generates a random number (`1 - 100`).
     - Connects to the Raspberry Pi server.
     - Sends the random number.
     - Displays the server's reply alongside its PID and child index.
   - The parent process uses `waitpid()` to coordinate and wait for all child processes to cleanly terminate.

## Files
- `server_challenge.c`: Concurrent server with semaphore thread pool and producer-consumer bounded queue.
- `client_challenge.c`: Multi-process client using `fork()` and random inputs.
- `Makefile`: Build script for both programs.

## Compilation
### On Raspberry Pi (Server):
```bash
gcc -o server_challenge server_challenge.c -lpthread
# Or:
make server_challenge
```

### On Ubuntu PC (Client):
```bash
gcc -o client_challenge client_challenge.c
# Or:
make client_challenge
```

## Running the Challenge

### Step 1: Start Server on Raspberry Pi
```bash
./server_challenge 12345
```

### Step 2: Run Multi-Process Client on Ubuntu PC
```bash
./client_challenge 192.168.3.1 12345
```
*(For local testing on one machine, run `./client_challenge localhost 12345`)*

### Example Output
#### Server Terminal:
```
===============================================================
 SC3103 Challenge Server started on port 12345
 Semaphore-managed Thread Pool: 3 concurrent worker threads
 Ready to serve up to 3 clients simultaneously!
===============================================================
[Worker 1] Initialized and waiting for client connections.
[Worker 2] Initialized and waiting for client connections.
[Worker 3] Initialized and waiting for client connections.

[Producer] New connection accepted (fd: 4). Enqueuing...
>>> [Worker 1] Serving client on socket descriptor 4...

[Producer] New connection accepted (fd: 5). Enqueuing...
>>> [Worker 2] Serving client on socket descriptor 5...

[Producer] New connection accepted (fd: 6). Enqueuing...
>>> [Worker 3] Serving client on socket descriptor 6...

<<< [Worker 1] Done serving socket 4: Input 42 -> Output 210
<<< [Worker 2] Done serving socket 5: Input 7 -> Output 35
<<< [Worker 3] Done serving socket 6: Input 89 -> Output 445
```

#### Client Terminal:
```
===============================================================
 SC3103 Challenge Client (Parent PID: 4501)
 Forking 3 concurrent child processes to test server
 Connecting to server at 192.168.3.1:12345
===============================================================

[Child 1 | PID 4502] Started. Generated random number: 42
[Child 2 | PID 4503] Started. Generated random number: 7
[Child 3 | PID 4504] Started. Generated random number: 89
>>> [Child 1 | PID 4502] SUCCESS! Sent 42 | Response: "Worker #1 processed: 42 * 5 = 210"
>>> [Child 2 | PID 4503] SUCCESS! Sent 7 | Response: "Worker #2 processed: 7 * 5 = 35"
>>> [Child 3 | PID 4504] SUCCESS! Sent 89 | Response: "Worker #3 processed: 89 * 5 = 445"
[Parent] Child 1 (PID 4502) finished with exit status 0.
[Parent] Child 2 (PID 4503) finished with exit status 0.
[Parent] Child 3 (PID 4504) finished with exit status 0.

===============================================================
 All 3 child client processes completed successfully!
===============================================================
```
