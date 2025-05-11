#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>

sem_t s;
int t_id[] = {1, 2, 3, 4, 5, 6};
void *t_func(void *arg);
int chips = 5;

int main() {
    pthread_t t[6];
    sem_init(&s, 0, 1);
    
    for (int i = 0; i < 6; i++) {
        pthread_create(&t[i], NULL, t_func, &t_id[i]);
    }
    
    for (int i = 0; i < 6; i++) {
        pthread_join(t[i], NULL);
    }
    
    sem_destroy(&s);
    return 0;
}

void *t_func(void *arg) {
    int id = *(int*)arg;
    
    sem_wait(&s);
    if (chips <= 0) {
        printf("No chips left for customer %d...\n", id);
        sem_post(&s);
        pthread_exit(NULL);
    }
    
    chips--; // Decrement immediately after check
    printf("Customer %d is eating chips...\n", id);
    sleep(1);
    printf("Customer %d has eaten a chip. Remaining chips: %d\n", id, chips);
    sem_post(&s);
    
    pthread_exit(NULL);
}