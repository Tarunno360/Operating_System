#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

int t_id[] = {1, 2};  // Thread identifiers
void *t_func1(void *id);
void *t_func2(void *id);

int sum = 15;  // Shared variable
pthread_mutex_t m;
sem_t s;

int main() {
    pthread_t t[2];

    // Initialize semaphore and mutex
    sem_init(&s, 0, 0);
    pthread_mutex_init(&m, NULL);

    // Create threads
    pthread_create(&t[0], NULL, t_func1, (void *)&t_id[0]);
    pthread_create(&t[1], NULL, t_func2, (void *)&t_id[1]);

    // Wait for threads to finish
    for (int i = 0; i < 2; i++) {
        pthread_join(t[i], NULL);
    }

    // Destroy semaphore and mutex
    sem_destroy(&s);
    pthread_mutex_destroy(&m);

    printf("Total sum: %d\n", sum);
    return 0;
}

void *t_func1(void *id) {
    sem_wait(&s);  // Wait until t_func2 signals
    pthread_mutex_lock(&m);

    printf("Thread %d started execution (Subtracting 10).\n", *(int *)id);
    for (int i = 0; i < 5; i++) {
        printf("Thread %d: Sum before  %d\n", *(int *)id, sum);
        sum -= 10;
        printf("Thread %d: Sum after : %d\n", *(int *)id, sum);
    }

    pthread_mutex_unlock(&m);
    return NULL;
}

void *t_func2(void *id) {
    pthread_mutex_lock(&m);

    printf("Thread %d started execution (Multiplying by 3).\n", *(int *)id);
    for (int i = 0; i < 5; i++) {
        printf("Thread %d: Sum before : %d\n", *(int *)id, sum);
        sum *= 3;
        printf("Thread %d: Sum after  %d\n", *(int *)id, sum);
    }

    pthread_mutex_unlock(&m);
    sem_post(&s);  // Signal t_func1 to proceed
    return NULL;
}
