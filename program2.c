#include <stdlib.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAXSIZE 1e9
int count;
int main()
{
    printf("Before fork\n");
    count = 0;
    pid_t child = fork();
    if (child == -1)
    {
        perror("FORK FAIL\n");
        exit(1);
    }
    if (child == 0)
    {
        int i;
        for (i = 0; i < MAXSIZE; i++)
        {
            count++;
        }
        printf("Child holding: %d\n", count);
    }
    else
    {
        wait(0);
        int i;
        for (i = 0; i < MAXSIZE; i++)
        {
            count++;
        }
        printf("Parent holding: %d\n", count);
    }
    return 0;
}