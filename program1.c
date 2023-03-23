#include <pthread.h>
#include <stdio.h>
#define MAXSIZE 1e9
int count;
pthread_mutex_t lock;
void *f_count(void *id)
{
    int i;
    // pthread_mutex_lock(&lock);
    for (i = 0; i < MAXSIZE; i++)
    {
        count++;
    }
    // pthread_mutex_unlock(&lock);
    printf("Thread %s holding: %d\n", (char *)id, count);
}
int main(int argc, char argv[])
{
    printf("Before thread\n");
    count = 0;
    pthread_t thread1, thread2;
    pthread_mutex_init(&lock, 0);
    pthread_create(&thread1, 0, &f_count, "1");
    pthread_create(&thread2, 0, &f_count, "2");
    pthread_join(thread1, 0);
    pthread_join(thread2, 0);
    printf("After thread\n");
    printf("Main holding: %d\n", count);
    return 0;
}
