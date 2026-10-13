# Practice B: Race Condition and Thread Synchronization

## Objectives
- Observe the behavior of multiple threads updating a shared variable without synchronization.
- Understand the concept of a **Race Condition**.

## Files
- `pthread2.c`: Source code with two threads `TA` and `TB` modifying `g_var1`.
- `Makefile`: Build automation file.

## Compilation
```bash
gcc -o pthread2 pthread2.c -lpthread
# Or using make:
make
```

## Execution
```bash
./pthread2
```

### Observed Output
You will observe non-deterministic outputs, repeated numbers, or unexpected values, such as:
```
 1 2 3 4 5 6 7 8 9 10 12 12 14 15 16 17 18 19 20
 pthread2 completed 
```
or out-of-order numbers.

---

## Analysis of Cause
1. **Shared Resource:** Both threads `TA` and `TB` concurrently read and write to the global variable `g_var1`.
2. **Non-Atomic Operation:** The increment operation `g_var1++` is not atomic at the machine-instruction level. It consists of three distinct steps:
   - Load `g_var1` from memory into a CPU register.
   - Increment the register value.
   - Store the register value back to memory.
3. **Context Switching & Interleaving:** Because there is no synchronization or lock, the OS scheduler can switch contexts between threads at any point (accentuated by the delay loop). If thread `TA` reads `g_var1`, and thread `TB` increments and writes it back before `TA` writes, `TA` will overwrite `TB`'s update with an outdated value. Furthermore, `printf()` is called after another delay, causing prints to observe corrupted or duplicated values.
4. **Resolution:** To prevent this, the shared variable update and printing must be enclosed in a **Critical Section** guarded by a mutual exclusion lock (Mutex), as implemented in Practice C.
