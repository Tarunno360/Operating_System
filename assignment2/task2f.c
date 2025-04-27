#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define max_number_student 10
#define total_chair 3

sem_t st_semaphore;         
sem_t student_semaphore;    
sem_t consultation_semaphore;  

pthread_mutex_t max_chairs_mutex;

int current_student_waiting = 0;
int current_total_served = 0;
int students_left_without_consultation = 0;

void* student_function(void* arg) {
    int id = *((int*)arg);
    free(arg);

    pthread_mutex_lock(&max_chairs_mutex);
    if (current_student_waiting < total_chair) {
        printf("Student %d started waiting for consultation\n", id);
        current_student_waiting++;

        if (current_student_waiting == 1) {
            printf("Student %d is waking up the ST!\n", id);
            sem_post(&st_semaphore); 
        }
        pthread_mutex_unlock(&max_chairs_mutex);

        sem_wait(&student_semaphore); 
        printf("Student %d is getting consultation\n", id);
        sleep(2); 

        printf("Student %d finished getting consultation and left\n", id);

        pthread_mutex_lock(&max_chairs_mutex);
        current_total_served++;
        printf("Number of served students: %d\n", current_total_served);
        pthread_mutex_unlock(&max_chairs_mutex);

        sem_post(&consultation_semaphore); 
    } else {
        printf("No chairs remaining in lobby. Student %d Leaving.....\n", id);
        pthread_mutex_lock(&max_chairs_mutex);
        students_left_without_consultation++;
        pthread_mutex_unlock(&max_chairs_mutex);
    }

    pthread_exit(NULL);
}

void* st_function(void* arg) {
    while (1) {
        printf("ST is going to sleep...\n");
        sem_wait(&st_semaphore); 
        printf("ST woke up to consult students!\n");

        while (1) {
            pthread_mutex_lock(&max_chairs_mutex);

            // Check if all students have been handled before serving a new one
            if (current_total_served + students_left_without_consultation >= max_number_student) {
                pthread_mutex_unlock(&max_chairs_mutex);
                pthread_exit(NULL);  // End ST thread immediately
            }

            if (current_student_waiting == 0) {
                pthread_mutex_unlock(&max_chairs_mutex);
                break;
            }

            sem_post(&student_semaphore);    
            printf("A waiting student started getting consultation\n");

            current_student_waiting--;
            printf("Number of students now waiting: %d\n", current_student_waiting);
            printf("ST giving consultation\n");
            pthread_mutex_unlock(&max_chairs_mutex);

            sleep(3); // Simulate consultation

            sem_wait(&consultation_semaphore); 
        }
    }
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
        usleep(rand() % 300000);  
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

    printf("\nAll students have been handled!\n");
    printf("Total students served: %d\n", current_total_served);
    printf("Total students left without consultation: %d\n", students_left_without_consultation);

    return 0;
}
