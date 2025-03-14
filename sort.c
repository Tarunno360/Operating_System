#include <stdio.h>
#include <stdlib.h>

// Comparator function for descending order sorting
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

    // Convert arguments to integers
    for (int i = 0; i < n; i++) {
        arr[i] = atoi(argv[i + 1]);
    }

    // Sort the array in descending order
    qsort(arr, n, sizeof(int), compare);

    // Print sorted array
    printf("Sorted array in descending order: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    return 0;
}

