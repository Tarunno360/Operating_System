#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TEMP_FILE "sorted_numbers.txt" 


int compare(const void *a, const void *b) {
    return (*(int *)b - *(int *)a);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <numbers>\n", argv[0]);
        return 1;
    }

    int n = argc - 1;
    int arr[n];

    for (int i = 0; i < n; i++) {
        arr[i] = atoi(argv[i + 1]);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }
    else if (pid == 0) {
        
        qsort(arr, n, sizeof(int), compare);

       
        FILE *fp = fopen(TEMP_FILE, "w");
        if (fp == NULL) {
            perror("Error opening file");
            exit(1);
        }
        
        printf("Sorted array in descending order (Child Process): ");
        for (int i = 0; i < n; i++) {
            printf("%d ", arr[i]);
            fprintf(fp, "%d\n", arr[i]); 
        }
        printf("\n");
        fclose(fp);

        exit(0); 
    }
    else {
        wait(NULL);

        FILE *fp = fopen(TEMP_FILE, "r");
        if (fp == NULL) {
            perror("Error opening file");
            return 1;
        }

        printf("Odd/Even status (Parent Process):\n");
        int num;
        while (fscanf(fp, "%d", &num) != EOF) {
            if (num % 2 == 0) {
                printf("%d is Even\n", num);
            } else {
                printf("%d is Odd\n", num);
            }
        }
        fclose(fp);

        
        remove(TEMP_FILE);
    }

    return 0;
}

