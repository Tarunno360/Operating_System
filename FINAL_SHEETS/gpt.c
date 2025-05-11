




// ✅ Example 1: Write Int Array as Text

#include <stdio.h>

int main() {
    int arr[] = {5, 10, 15, 20, 25};
    int size = sizeof(arr) / sizeof(arr[0]);

    FILE *file = fopen("output.txt", "w");
    if (file == NULL) {
        printf("Failed to open file.\n");
        return 1;
    }

    for (int i = 0; i < size; i++) {
        fprintf(file, "%d ", arr[i]);  // Write each int followed by space
    }

    fclose(file);
    return 0;
}


//✅ Using fscanf() (Preferred for reading integers directly from file)

#include <stdio.h>

int main() {
    FILE *file = fopen("numbers.txt", "r");
    if (file == NULL) {
        printf("Failed to open file.\n");
        return 1;
    }

    int num;
    while (fscanf(file, "%d", &num) == 1) {  // Read integers one by one
        printf("%d\n", num);  // Print the integer
    }

    fclose(file);
    return 0;
}


//✅ Example 2: Using fgets() and atoi() (if integers are stored as strings)

#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *file = fopen("numbers.txt", "r");
    if (file == NULL) {
        printf("Failed to open file.\n");
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        int num = atoi(line);  // Convert string to integer
        printf("%d\n", num);
    }

    fclose(file);
    return 0;
}




