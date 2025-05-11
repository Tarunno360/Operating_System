//WRITE ON FILE GIVEN FROM TERMINAL

#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "w");
    if (file == NULL) {
        printf("Could not open file.\n");
        return 1;
    }

    fprintf(file, "Written to file: %s\n", argv[1]);
    fclose(file);

    printf("Done writing to %s\n", argv[1]);
    return 0;
}



// Taking INTEGERS FROM TERMINAL AND MAKING ARRAY

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Usage: %s num1 num2 num3 num4 num5\n", argv[0]);
        return 1;
    }

    int arr[5];
    for (int i = 0; i < 5; i++) {
        arr[i] = atoi(argv[i + 1]);  // Convert string to int
    }

    printf("You entered:\n");
    for (int i = 0; i < 5; i++) {
        printf("arr[%d] = %d\n", i, arr[i]);
    }

    return 0;
}



// NUMBER OF INTEGERS UNKNOWN FOR ABOVE PROBLEM

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s num1 [num2 ... numN]\n", argv[0]);
        return 1;
    }

    int count = argc - 1;
    int arr[count];

    for (int i = 0; i < count; i++) {
        arr[i] = atoi(argv[i + 1]);  // Convert each argument to int
    }

    printf("You entered %d numbers:\n", count);
    for (int i = 0; i < count; i++) {
        printf("arr[%d] = %d\n", i, arr[i]);
    }

    return 0;
}










