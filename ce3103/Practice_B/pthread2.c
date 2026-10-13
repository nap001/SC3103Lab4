//******************************************************************
// pthread2.c   CE3103/SC3103 Exercise #4 Practice B
// Demonstrating Race Condition and Lack of Thread Synchronization
//
// Reference: Section 2.2 of SC3103 Lab 4 Manual
// Compilation: gcc -o pthread2 pthread2.c -lpthread
// Execution:   ./pthread2
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int g_var1 = 0;    // Shared global variable

void *inc_gv() 
{ 
    int i, j; 
    for (i = 0; i < 10; i++) 
    { 
        g_var1++;                   // increment the global variable (RACE CONDITION)
        for (j = 0; j < 5000000; j++);   // delay loop to increase context switch probability
        printf(" %d", g_var1);      // print the value
        fflush(stdout); 
    } 
    return NULL;
} 

int main()
{ 
    pthread_t TA, TB;
    int TAret, TBret; 

    /* Spawn two threads that concurrently update and print g_var1 */
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

    printf("\n pthread2 completed \n"); 
    return 0;
}
