//******************************************************************
// mutex1.c   CE3103/SC3103 Exercise #4 Practice C
// Resolving Race Condition using Mutex (Critical Section)
//
// Reference: Section 2.3 of SC3103 Lab 4 Manual
// Compilation: gcc -o mutex1 mutex1.c -lpthread
// Execution:   ./mutex1
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int g_var1 = 0;              // Shared global variable
pthread_mutex_t mutexA;      // Mutex object to delimit critical section

void *inc_gv() 
{ 
    int i, j; 
    for (i = 0; i < 10; i++) 
    { 
        /* Enter Critical Section */
        pthread_mutex_lock(&mutexA);

        g_var1++;                   // Atomically increment the global variable
        for (j = 0; j < 5000000; j++);   // Delay loop
        printf(" %d", g_var1);      // Print the value
        fflush(stdout); 

        /* Exit Critical Section */
        pthread_mutex_unlock(&mutexA);
    } 
    return NULL;
} 

int main()
{ 
    pthread_t TA, TB;
    int TAret, TBret; 

    /* Initialize the mutex object */
    pthread_mutex_init(&mutexA, NULL);

    /* Spawn two threads */
    TAret = pthread_create(&TA, NULL, inc_gv, NULL);
    TBret = pthread_create(&TB, NULL, inc_gv, NULL);

    if (TAret != 0 || TBret != 0) 
    {
        fprintf(stderr, "Error creating threads\n");
        return 1;
    }

    /* Wait for both threads to finish */
    pthread_join(TA, NULL);
    pthread_join(TB, NULL);

    /* Clean up / destroy the mutex object */
    pthread_mutex_destroy(&mutexA);

    printf("\n mutex1 completed \n"); 
    return 0;
}
