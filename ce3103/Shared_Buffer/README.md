# SC3103 / CE3103 - Shared Circular Buffer (Client-Server IPC)

This folder implements the **Shared Buffer** problem using C, POSIX sockets, multi-process management (`fork()`), and multithreaded synchronization (`pthreads` with mutexes/semaphores).

---

## 1. System Overview

```
 [ Raspberry Pi: Client Program ]                         [ PC: Server Program ]
  Parent Process (PID P)                                   Main Thread (Listening on TCP port)
    ├── fork() Child 1 (PID C1) ─── TCP Connect ───► Worker Thread 1 ──┐
    ├── fork() Child 2 (PID C2) ─── TCP Connect ───► Worker Thread 2 ──┤ Critical Section
    ├── ...                     ─── TCP Connect ───► ...             ──┼ (Mutex / Semaphore)
    └── fork() Child N (PID CN) ─── TCP Connect ───► Worker Thread N ──┘        │
                                                                                ▼
                                                                     [ Shared Circular Buffer ]
                                                                     - Slots: size 5
                                                                     - write_index: 0..4
                                                                     - sum: running sum
```

### Specifications:
1. **Client (for Raspberry Pi):**
   - Takes command-line argument `N`.
   - Uses `fork()` to spawn `N` concurrent child processes.
   - Each child establishes a TCP connection to the server.
   - Sends one integer to the server.
   - Receives server reply.
   - Prints: `PID`, `value sent`, `buffer slot index`, `current buffer sum`.
   - Parent process waits for all `N` children using `wait()`.

2. **Server (for PC):**
   - Listens on a TCP port.
   - Creates a dedicated thread (`pthread_create`) per incoming client connection.
   - Maintains a shared circular buffer (capacity of 5), a `write_index`, and the `sum`.
   - Synchronizes access via a critical section protected by a **Mutex** (or **Semaphore**).
   - In the critical section:
     - Inserts the received integer into `buffer[write_index]`.
     - Advances the circular index: `write_index = (write_index + 1) % 5`.
     - Updates the running buffer `sum` (subtracting the overwritten value, adding the new value).
     - Prints: `value`, `slot index`, `buffer sum`, and current buffer contents.
   - Sends back: `value`, `slot index`, `buffer sum`.

---

## 2. Compilation

Compile both client and server binaries using `make`:

```bash
# Default build (Mutex-based critical section)
make

# Optional: Build server using POSIX Semaphore (sem_t) instead of Mutex
make clean
make USE_SEMAPHORE=1
```

To clean up binary executables:
```bash
make clean
```

---

## 3. Execution Guide

### Step 1: Start Server on PC
```bash
# Run on default port 12345
./server

# Or specify a custom port
./server 12345
```

### Step 2: Run Client on Raspberry Pi (or PC)
```bash
# General syntax:
./client <server_ip> <port> <N>

# Example connecting from RPi to PC server:
./client 192.168.1.100 12345 7

# For testing locally on the same PC:
./client 7
```

---

## 4. Example Execution Trace

### PC Server Terminal:
```
===============================================================
 SC3103 Shared Circular Buffer Server (PC)
 Listening on Port: 12345
 Buffer Size:       5 slots (circular overwrite)
 Synchronization:   POSIX Mutex (pthread_mutex_t)
 Waiting for RPi client connections... (Press Ctrl+C to stop)
===============================================================
[Server Thread 139782] Received: 42 | Slot Index: 0 | Buffer Sum: 42  | State: [42, 0, 0, 0, 0]   | Next Slot: 1
[Server Thread 139783] Received: 15 | Slot Index: 1 | Buffer Sum: 57  | State: [42, 15, 0, 0, 0]  | Next Slot: 2
[Server Thread 139784] Received: 88 | Slot Index: 2 | Buffer Sum: 145 | State: [42, 15, 88, 0, 0] | Next Slot: 3
[Server Thread 139785] Received:  7 | Slot Index: 3 | Buffer Sum: 152 | State: [42, 15, 88, 7, 0] | Next Slot: 4
[Server Thread 139786] Received: 23 | Slot Index: 4 | Buffer Sum: 175 | State: [42, 15, 88, 7, 23]| Next Slot: 0
[Server Thread 139787] Received: 60 | Slot Index: 0 | Buffer Sum: 193 | State: [60, 15, 88, 7, 23]| Next Slot: 1
[Server Thread 139788] Received: 10 | Slot Index: 1 | Buffer Sum: 188 | State: [60, 10, 88, 7, 23]| Next Slot: 2
```
*(Notice how on the 6th insertion, slot 0 is overwritten from 42 to 60, updating sum from 175 - 42 + 60 = 193)*

### RPi Client Terminal (`N = 7`):
```
===============================================================
 SC3103 Shared Circular Buffer Client (RPi)
 Target Server: 192.168.1.100:12345
 Spawning N = 7 child processes via fork()...
===============================================================

[Child PID 41201] Sent Value: 42 | Buffer Slot Index: 0 | Current Buffer Sum: 42
[Child PID 41202] Sent Value: 15 | Buffer Slot Index: 1 | Current Buffer Sum: 57
[Child PID 41203] Sent Value: 88 | Buffer Slot Index: 2 | Current Buffer Sum: 145
[Child PID 41204] Sent Value:  7 | Buffer Slot Index: 3 | Current Buffer Sum: 152
[Child PID 41205] Sent Value: 23 | Buffer Slot Index: 4 | Current Buffer Sum: 175
[Child PID 41206] Sent Value: 60 | Buffer Slot Index: 0 | Current Buffer Sum: 193
[Child PID 41207] Sent Value: 10 | Buffer Slot Index: 1 | Current Buffer Sum: 188

===============================================================
 [Parent PID 41200] All 7 child processes completed successfully.
===============================================================
```

---

## 5. Technical Highlights & Concurrency Details

1. **Race Condition Prevention on Client Socket Descriptors:**
   - The server dynamically allocates `int *client_sock = malloc(sizeof(int))` for each `accept()` connection before passing it to `pthread_create()`. This prevents subsequent `accept()` calls from mutating the descriptor before the thread reads it.
2. **Mutual Exclusion:**
   - All access to `shared.buffer`, `shared.write_index`, and `shared.sum` is strictly encapsulated inside `pthread_mutex_lock()` and `pthread_mutex_unlock()` (or `sem_wait()` / `sem_post()`).
3. **Circular Overwrite Math:**
   - Overwrite calculation correctly subtracts `old_val` before adding `new_val`:
     `sum = sum - old_val + value;`
     `write_index = (write_index + 1) % 5;`
4. **Clean Parent Termination:**
   - The parent client process invokes `wait()` in a loop to collect all children, eliminating zombie processes.
