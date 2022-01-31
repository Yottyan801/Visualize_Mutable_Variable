#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_CNT 10000
int grobal_cnt = 0;
pthread_mutex_t mutex1, mutex2;
void *thread1(void *);
void *thread2(void *);

int main(void)
{
    int i = 0;
    pthread_t tid1, tid2;
    void *thread_return;

    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);

    pthread_create(&tid1, NULL, thread1, NULL);
    pthread_create(&tid2, NULL, thread2, NULL);

    pthread_join(tid1, &thread_return);
    pthread_join(tid2, &thread_return);

    printf("grobal_cnt = %d\n", grobal_cnt);
    return 0;
}
void *thread1(void *arg)
{
    int i = 0;
    pthread_mutex_lock(&mutex1);
    sleep(1);
    pthread_mutex_lock(&mutex2);
    for (i = 0; i < MAX_CNT; i++)
        grobal_cnt++;
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);

    pthread_exit(0);
}

void *thread2(void *arg)
{
    int i = 0;
    pthread_mutex_lock(&mutex2);
    sleep(1);
    pthread_mutex_lock(&mutex1);
    for (i = 0; i < MAX_CNT; i++)
        grobal_cnt++;
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);

    pthread_exit(0);
}