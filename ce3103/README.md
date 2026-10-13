# SC3103 / CE3103 - Practical Exercise #4: Multithreaded C Programming

**Embedded Platform:** Raspberry Pi 3/4 (Raspbian OS) & Ubuntu Desktop PC  
**Course:** SC3103 Embedded Programming, NTU CCDS  

---

## Directory & File Structure

As outlined by the laboratory manual (Section 2.1: `ce3103/<labgroup>/`), the solutions are cleanly modularized into separate exercise folders:

```
Lab 4/
├── Makefile                          # Root Makefile building all exercises
├── README.md                         # Complete project documentation
└── ce3103/
    ├── Makefile                      # Top-level makefile for all subdirectories
    ├── README.md                     # Lab guide and theory notes
    │
    ├── Practice_A/                   # Section 2.1: Simple two-thread C program
    │   ├── pthread1.c                # Appendix A implementation
    │   ├── Makefile                  # Build script (gcc -lpthread)
    │   └── README.md                 # Execution instructions & analysis
    │
    ├── Practice_B/                   # Section 2.2: Race condition demonstration
    │   ├── pthread2.c                # Unsynchronized shared variable modification
    │   ├── Makefile                  # Build script
    │   └── README.md                 # Detailed race condition explanation
    │
    ├── Practice_C/                   # Section 2.3: Critical section & Mutex
    │   ├── mutex1.c                  # Thread synchronization using pthread_mutex_t
    │   ├── Makefile                  # Build script
    │   └── README.md                 # Verification and explanation
    │
    ├── Exercise/                     # Section 3.0: Client-Server Socket IPC
    │   ├── server.c                  # RPi multithreaded server (multiplies by 5)
    │   ├── client.c                  # Ubuntu client requesting user number
    │   ├── Makefile                  # Build script for server & client
    │   └── README.md                 # Network setup & execution guide
    │
    ├── Challenge/                    # Section 3 Challenge
    │   ├── server_challenge.c        # Up to 3 simultaneous threads with Producer-Consumer semaphore scheme
    │   ├── client_challenge.c        # Client forking 3 child processes with random inputs
    │   ├── Makefile                  # Build script for challenge binaries
    │   └── README.md                 # Challenge architecture & sample run
    │
    └── Shared_Buffer/                # Shared Circular Buffer (Client-Server IPC)
        ├── server.c                  # PC multithreaded server (circular buffer size 5 + mutex/sem)
        ├── client.c                  # RPi client forking N children sending integers
        ├── Makefile                  # Build script
        └── README.md                 # Execution instructions & sample trace
```

---

## Exercise Summary & Concepts

### 1. Practice A: Two-Thread Pthreads (`ce3103/Practice_A/`)
- **Key Concepts:** `pthread_create()`, `pthread_join()`, passing arguments to thread routines.
- **Manual Reflection Question:** *What happens when `pthread_join()` is commented out?*
  - The `main()` thread immediately hits the process exit (`return 0`), which tears down the entire address space before child threads finish executing or printing.

### 2. Practice B: Race Condition (`ce3103/Practice_B/`)
- **Key Concepts:** Shared memory concurrency without synchronization.
- **Observation:** `g_var1++` is a non-atomic read-modify-write operation across threads `TA` and `TB`. Interleaved execution and the delay loop cause corrupted values, missed updates, and repeated numbers.

### 3. Practice C: Mutex & Critical Section (`ce3103/Practice_C/`)
- **Key Concepts:** `pthread_mutex_t`, `pthread_mutex_lock()`, `pthread_mutex_unlock()`.
- **Solution:** Enclosing the variable increment and output inside a critical section guarantees mutual exclusion, yielding sequential outputs from `1` through `20`.

### 4. Section 3 Exercise: Client-Server Network IPC (`ce3103/Exercise/`)
- **Key Concepts:** TCP stream sockets (`AF_INET`, `SOCK_STREAM`), socket binding, listening, accepting, and thread creation per connection (`pthread_create()`).
- **Functionality:**
  - `client.c`: Runs on Ubuntu PC, connects to `192.168.3.1:port`, prompts for a number, sends it over TCP, prints response.
  - `server.c`: Runs on RPi, accepts connection, spawns detached worker thread to multiply the number by 5 and return the formatted answer.

### 5. Challenge: Producer-Consumer Semaphore Scheme & Multi-Process Client (`ce3103/Challenge/`)
- **Server Challenge (`server_challenge.c`):**
  - References Lecture Notes **W8L2 Slides 45-52**.
  - Limits server concurrency strictly to **3 threads serving up to 3 clients simultaneously**.
  - Employs a bounded circular queue (`buf[3]`) managed by two counting semaphores (`sem_buffer` initialized to 3, `sem_data` initialized to 0) and a mutex (`pthread_mutex_t`).
  - Main thread is the **Producer** (enqueues accepted sockets).
  - 3 worker threads are **Consumers** (dequeue sockets, compute `number * 5`, reply, close socket).
- **Client Challenge (`client_challenge.c`):**
  - References Lecture Notes **W9L3 Slides 46-47** and **W8L2 Slide 55**.
  - Uses `fork()` to spawn **3 concurrent child processes**.
  - Each child process generates a random number, establishes connection to the server, receives the calculated response, and prints results with its PID.
  - Parent process uses `waitpid()` to coordinate child termination.

---

## Build & Test Instructions

### Quick Build All
```bash
cd "ce3103"
make all
```

### Clean All Binaries
```bash
make clean
```
