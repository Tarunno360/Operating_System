#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int *sequence_capture_arr = NULL;
int num_of_terms = 0;

int *searching_index = NULL;
int num_search = 0;

void *fibonacci_sequence_generation(void *arg) {
    sequence_capture_arr = (int *)malloc((num_of_terms + 1) * sizeof(int));
    if (sequence_capture_arr == NULL) {
        perror("Sorry. Array is empty");
        pthread_exit(NULL);
    }

    sequence_capture_arr[0] = 0;
    if (num_of_terms > 0) sequence_capture_arr[1] = 1;

    for (int i = 2; i <= num_of_terms; ++i) {
        sequence_capture_arr[i] = sequence_capture_arr[i - 1] + sequence_capture_arr[i - 2];
    }

    pthread_exit(NULL);
}


int main() {
    pthread_t fib_thread, search_thread;

    printf("Enter the term of Fibonacci sequence:\n");
    scanf("%d", &num_of_terms);

    pthread_create(&fib_thread, NULL, fibonacci_sequence_generation, NULL);
    pthread_join(fib_thread, NULL);

    for (int i = 0; i <= num_of_terms; ++i) {
        printf("a[%d] = %d\n", i, sequence_capture_arr[i]);
    }
return 0;
}