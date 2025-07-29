#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

int id[] = {1, 2};

void *t_func1(int *id);
void *t_func2(int *id);

int sum = 15;
pthread_mutex_t m;
sem_t s;

int main() {
    pthread_t t[2];
    sem_init(&s, 0, 0);
    pthread_mutex_init(&m, NULL);
    pthread_create(&t[0], NULL, (void *)t_func1, &id[0]);
    pthread_create(&t[1], NULL, (void *)t_func2, &id[1]);

    for (int i = 0; i < 2; i++) {
        pthread_join(t[i], NULL);
    }

    sem_destroy(&s);
    pthread_mutex_destroy(&m);
    printf("Total sum: %d\n", sum);
    return 0;
}

void *t_func1(int *id) {
    sem_wait(&s);
    pthread_mutex_lock(&m);
    for (int i = 0; i < 5; i++) {
        printf("Sum: %d\n", sum);
        sum -= 10;
    }
    pthread_mutex_unlock(&m);
    sem_post(&s);
}

void *t_func2(int *id) {
    pthread_mutex_lock(&m);
    for (int i = 0; i < 5; i++) {
        printf("Sum: %d\n", sum);
        sum *= 3;
    }
    pthread_mutex_unlock(&m);
    sem_post(&s);
}