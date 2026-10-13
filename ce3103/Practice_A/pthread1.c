//******************************************************************
// pthread1.c   CE3103/SC3103 Exercise #4 Practice A sample program
// This program will create two threads, each prints its
// own message onto the screen.
//
// Reference: Appendix A of SC3103 Lab 4 Manual
// Compilation: gcc -o pthread1 pthread1.c -lpthread
// Execution:   ./pthread1
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Thread function that prints the message passed via argument */
void *print_message(void *ptr)
{
    char *text;
    text = (char *) ptr;
    printf("%s \n", text);
    return NULL;
}

int main()
{
    pthread_t thread1, thread2;
    char *str1 = "I am thread 1";
    char *str2 = "I am thread 2";
    int T1ret, T2ret;

    /* Create two threads */
    T1ret = pthread_create(&thread1, NULL, print_message, (void *) str1);
    T2ret = pthread_create(&thread2, NULL, print_message, (void *) str2);

    /* 
     * main() thread now waits for both threads to finish before exiting.
     * Explore further question from manual:
     * If these two pthread_join() statements are commented out, the main thread 
     * terminates immediately and exits the process, which can kill the spawned
     * threads before they have a chance to finish printing their messages.
     */
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("T1 & T2 return: %d, %d\n", T1ret, T2ret);
    return 0;
}
