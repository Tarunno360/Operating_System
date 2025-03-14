#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char *argv[]) {
        if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "a");

    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    char userInput[256];
    while (1) {
        printf("Enter a string to write to the file (or -1 to stop): ");
        fgets(userInput, sizeof(userInput), stdin);

        userInput[strcspn(userInput, "\n")] = '\0';

        if (strcmp(userInput, "-1") == 0) {
            break;
        }

        fprintf(file, "%s\n", userInput);
    }

    fclose(file);

    printf("Data has been written to %s successfully.\n", argv[1]);

    return 0;
}
