#include <stdio.h>
#include <pthread.h>

void f1(void);
void f2(void);
int counter = 0;
int main(void)
{
    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, (void *)f1, NULL);
    pthread_create(&thread2, NULL, (void *)f2, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}

void f1(void)
{
    int i;
    for (i = 0; i < 10; i++)
    {
        counter++;
        printf("counter = %d\n", counter);
    }
}

void f2(void)
{
    int i;
    for (i = 0; i < 10; i++)
    {
        counter++;
    }
}