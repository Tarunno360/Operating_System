#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int *sequence_capture_arr = NULL;
int num_of_terms = 0;

int *searching_index = NULL;
int num_searched_value = 0;

void *fibonacci_sequence_generation(void *arg) {
    sequence_capture_arr = (int *)malloc((num_of_terms + 1) * sizeof(int));
    sequence_capture_arr[0] = 0;
    if (num_of_terms > 0) 
        sequence_capture_arr[1] = 1;
    for (int i = 2; i <= num_of_terms; ++i) {
        sequence_capture_arr[i] = sequence_capture_arr[i - 1] + sequence_capture_arr[i - 2];
    }

    pthread_exit(NULL);
}
void *fibonacci_value_search(void *arg) {
    for (int i = 0; i < num_searched_value; ++i) {
        int index = searching_index[i];
        if (index >= 0 && index <= num_of_terms) {
            printf("result of search #%d = %d\n", i + 1, sequence_capture_arr[index]);
        } else {
            printf("result of search #%d = -1\n", i + 1);
        }
    }
    pthread_exit(NULL);
}


int main() {
    pthread_t fib_thread, search_thread;

    printf("Enter the term of Fibonacci sequence:\n");
    scanf("%d", &num_of_terms);

    while (num_of_terms < 0 || num_of_terms > 40) {
        printf("Your number is out of range. Please enter a number between 0 and 40:\n");
        scanf("%d", &num_of_terms);
    }

    pthread_create(&fib_thread, NULL, fibonacci_sequence_generation, NULL);
    pthread_join(fib_thread, NULL);

    for (int i = 0; i <= num_of_terms; ++i) {
        printf("a[%d] = %d\n", i, sequence_capture_arr[i]);
    }
    printf("How many numbers you are willing to search?:\n");
    scanf("%d", &num_searched_value);

    while (num_searched_value <= 0 ) {
        printf("Invalid input. Please enter a number greater than 0:\n");
        scanf("%d", &num_searched_value);
    }

    searching_index = (int *)malloc(num_searched_value * sizeof(int));

    for (int i = 0; i < num_searched_value; ++i) {
        printf("Enter search %d:\n", i + 1);
        scanf("%d", &searching_index[i]);
    }

    pthread_create(&search_thread, NULL, fibonacci_value_search, NULL);
    pthread_join(search_thread, NULL);

return 0;
}