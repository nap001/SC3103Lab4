# Practice C: Critical Section and Mutex

## Objectives
- Eliminate race conditions using a POSIX Mutex (`pthread_mutex_t`).
- Protect shared resources within a defined **Critical Section**.

## Files
- `mutex1.c`: Source code using `pthread_mutex_lock()` and `pthread_mutex_unlock()`.
- `Makefile`: Build automation file.

## POSIX Mutex Functions Used
| Function | Purpose |
|---|---|
| `pthread_mutex_init(&mutexA, NULL)` | Initialize the mutex object with default attributes |
| `pthread_mutex_lock(&mutexA)` | Acquire the mutex lock (blocks if already locked by another thread) |
| `pthread_mutex_unlock(&mutexA)` | Release the mutex lock to allow other threads to enter |
| `pthread_mutex_destroy(&mutexA)` | Deallocate the mutex object |

## Compilation
```bash
gcc -o mutex1 mutex1.c -lpthread
# Or using make:
make
```

## Execution
```bash
./mutex1
```

### Expected Output
```
 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
 mutex1 completed 
```

## Conclusion
With the mutex lock placed around the increment, delay loop, and print statement, each thread safely completes an entire atomic update and display cycle without interruption or corruption from the other thread. The values increment predictably from 1 through 20.
