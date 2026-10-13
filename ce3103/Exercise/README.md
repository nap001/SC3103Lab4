# Section 3 Exercise: Client-Server Socket Communication

## Overview
This exercise implements network inter-process communication (IPC) across machines using TCP/IP sockets and multithreading:
1. **Client (`client.c`):** Runs on the Ubuntu computer. Prompts the user to enter a number, transmits it over the network to the server, and prints the server's reply.
2. **Server (`server.c`):** Runs on the Raspberry Pi board (IP `192.168.3.1`). Listens for connections and spawns a thread (`pthread_create`) for each incoming client. The thread multiplies the received number by 5 and returns the response.

## Files
- `server.c`: Multithreaded TCP server.
- `client.c`: TCP client with interactive input.
- `Makefile`: Automates building both programs.

## Compilation
### On Raspberry Pi (Server):
```bash
gcc -o server server.c -lpthread
# Or:
make server
```

### On Ubuntu PC (Client):
```bash
gcc -o client client.c
# Or:
make client
```

## Running the Programs

### Step 1: Start Server on Raspberry Pi
Open a terminal on the Raspberry Pi (via SSH: `ssh pi@192.168.3.1`):
```bash
./server 12345
```

### Step 2: Run Client on Ubuntu PC
In another terminal on the Ubuntu PC:
```bash
./client 192.168.3.1 12345
```
*(If testing on the same machine, use `localhost` or `127.0.0.1` as the IP: `./client localhost 12345`)*

### Interaction Flow Example
1. Client prompts:
   ```
   Connecting to server at 192.168.3.1:12345...
   Successfully connected to server.
   Please enter a number: 14
   ```
2. Server console displays:
   ```
   [Main Thread] Accepted new connection (fd: 4).
   [Server Thread 140324859012864] Received number/message: 14
   [Server Thread 140324859012864] Replied: Server result: 70 (original 14 * 5)
   ```
3. Client console displays:
   ```
   Message received from server: Server result: 70 (original 14 * 5)
   ```
