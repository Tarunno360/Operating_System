#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_STUDENTS 10
#define MAX_CHAIRS 3

sem_t st_sleeping;          // Semaphore to wake up ST
sem_t student_ready;        // Semaphore for students waiting
pthread_mutex_t mutex;      // Mutex for critical sections

int waiting_students = 0;
int students_served = 0;

void* student_thread(void* arg) {
    int id = *(int*)arg;
    free(arg); // Free the allocated memory

    sleep(rand() % 3);  // Random arrival

    pthread_mutex_lock(&mutex);
    if (waiting_students < MAX_CHAIRS) {
        printf("Student %d started waiting for consultation\n", id);
        waiting_students++;
        printf("Number of students now waiting: %d\n", waiting_students);

        sem_post(&st_sleeping); // Wake up the ST if sleeping
        pthread_mutex_unlock(&mutex);

        sem_wait(&student_ready); // Wait for ST to be ready

        printf("Student %d is getting consultation\n", id);
        sleep(1);  // Consultation time

        pthread_mutex_lock(&mutex);
        waiting_students--;
        students_served++;
        printf("Student %d finished getting consultation and left\n", id);
        printf("Number of served students: %d\n", students_served);
        pthread_mutex_unlock(&mutex);

    } else {
        printf("No chairs remaining in lobby. Student %d Leaving.....\n", id);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

void* st_thread(void* arg) {
    while (students_served < NUM_STUDENTS) {
        sem_wait(&st_sleeping);  // Sleep until a student wakes up

        pthread_mutex_lock(&mutex);
        if (waiting_students > 0) {
            printf("A waiting student started getting consultation\n");
            printf("Number of students now waiting: %d\n", waiting_students);
        }
        pthread_mutex_unlock(&mutex);

        sem_post(&student_ready); // Allow student to get consultation
        printf("ST giving consultation\n");

        sleep(1); // ST consult time
    }

    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    pthread_t st, students[NUM_STUDENTS];

    // Initialize semaphores and mutex
    sem_init(&st_sleeping, 0, 0);
    sem_init(&student_ready, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    // Create ST thread
    pthread_create(&st, NULL, st_thread, NULL);

    // Create student threads
    for (int i = 0; i < NUM_STUDENTS; i++) {
        int* id = malloc(sizeof(int));
        *id = i;
        pthread_create(&students[i], NULL, student_thread, id);
    }

    // Join all student threads
    for (int i = 0; i < NUM_STUDENTS; i++) {
        pthread_join(students[i], NULL);
    }

    // ST thread may finish late
    pthread_join(st, NULL);

    // Cleanup
    sem_destroy(&st_sleeping);
    sem_destroy(&student_ready);
    pthread_mutex_destroy(&mutex);

    return 0;
}
