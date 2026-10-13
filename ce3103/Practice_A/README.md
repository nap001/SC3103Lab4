# Practice A: Simple Two-Thread C Program

## Objectives
- Learn how to create and spawn threads using POSIX threads (`pthread`).
- Understand thread termination and synchronization using `pthread_join()`.

## Files
- `pthread1.c`: Source code creating two threads executing `print_message()`.
- `Makefile`: Build automation file.

## Compilation
On Raspberry Pi / Linux terminal:
```bash
gcc -o pthread1 pthread1.c -lpthread
# Or using make:
make
```

## Execution
```bash
./pthread1
```

### Expected Output
```
I am thread 1 
I am thread 2 
T1 & T2 return: 0, 0
```
*(Note: Output order between thread 1 and thread 2 depends on OS scheduler).*

---

## Lab Question / Explore Further
**Question:**
> *Comment off the two `pthread_join()` statements in the program. Compile and run the program, and observe whether it still behaves as expected.*

**Explanation:**
- When `pthread_join(thread1, NULL)` and `pthread_join(thread2, NULL)` are commented out, the `main()` thread executes straight to `return 0;`.
- Calling `return 0` in `main()` terminates the entire process and deallocates its address space immediately.
- Consequently, the spawned worker threads are terminated before they have an opportunity to finish (or even begin) executing `print_message()`.
- You may observe that neither or only one of the thread messages prints to the terminal.
