#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
sem_t st_semaphore;         
sem_t student_semaphore;      
#define max_number_student 10
#define total_chair 3
int current_student_waiting = 0;
int current_total_surved = 0;
pthread_mutex_t max_chairs_mutex;
sem_t consultation_semaphore;  

void* student_function(void* arg) {
    int id = *((int*)arg);
    free(arg);

    pthread_mutex_lock(&max_chairs_mutex);
    if (current_student_waiting < total_chair) {
        printf("Student %d started waiting for consultation\n", id);
        current_student_waiting++;

        if (current_student_waiting == 1) {
            sem_post(&st_semaphore); 
        }

        pthread_mutex_unlock(&max_chairs_mutex);

        sem_wait(&student_semaphore); 
        printf("Student %d is getting consultation\n", id);
        sleep(2); 

        printf("Student %d finished getting consultation and left\n", id);

        pthread_mutex_lock(&max_chairs_mutex);
        current_total_surved++;
        printf("Number of served students: %d\n", current_total_surved);
        pthread_mutex_unlock(&max_chairs_mutex);

        sem_post(&consultation_semaphore); 
    } else {
        printf("No chairs remaining in lobby. Student %d Leaving.....\n", id);
        pthread_mutex_unlock(&max_chairs_mutex);
    }

    pthread_exit(NULL);
}
void* st_function(void* arg) {
    while (1) {
        sem_wait(&st_semaphore); 

        while (1) {
            pthread_mutex_lock(&max_chairs_mutex);
            if (current_student_waiting == 0) {
                pthread_mutex_unlock(&max_chairs_mutex);
                break;
            }

        sem_post(&student_semaphore);    
        printf("A waiting student started getting consultation\n");

        current_student_waiting--;
        printf("Number of students now waiting: %d\n", current_student_waiting);
        printf("ST giving consultation\n");
        sleep(3);
        pthread_mutex_unlock(&max_chairs_mutex);
        sem_wait(&consultation_semaphore); 
        }

        
        pthread_mutex_lock(&max_chairs_mutex);
        if (current_total_surved >= max_number_student) {
            pthread_mutex_unlock(&max_chairs_mutex);
            break;
        }
        pthread_mutex_unlock(&max_chairs_mutex);
    }

    pthread_exit(NULL);
}
int main() {
    pthread_t st_thread;
    pthread_t student_threads[max_number_student];

    sem_init(&st_semaphore, 0, 0);
    sem_init(&student_semaphore, 0, 0);
    sem_init(&consultation_semaphore, 0, 0);
    pthread_mutex_init(&max_chairs_mutex, NULL);

    pthread_create(&st_thread, NULL, st_function, NULL);

    for (int i = 0; i < max_number_student; ++i) {
        int* id = malloc(sizeof(int));
        *id = i;
        //usleep(rand() % 900000);  
        usleep(10000);
        pthread_create(&student_threads[i], NULL, student_function, id);
    }

    for (int i = 0; i < max_number_student; ++i) {
        pthread_join(student_threads[i], NULL);
    }

    pthread_join(st_thread, NULL);

    sem_destroy(&st_semaphore);
    sem_destroy(&student_semaphore);
    sem_destroy(&consultation_semaphore);
    pthread_mutex_destroy(&max_chairs_mutex);
    return 0;
}