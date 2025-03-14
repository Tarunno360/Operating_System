#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Please enter with : %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "w");
    
    char userInput[256];

    while (1) {
        printf("Enter a string to write to your selected file (if you want to stop press: -1): ");
        fgets(userInput, sizeof(userInput), stdin);
        int i = 0;
        while (userInput[i] != '\0') {
            if (userInput[i] == '\n') {
                userInput[i] = '\0';
                break;
            }
            i++;
        }
        if (userInput[0] == '-' && userInput[1] == '1' && userInput[2] == '\0') {
            break;
        }

        fprintf(file, "%s\n", userInput);
    }

    fclose(file);
    printf("Data has been written to %s successfully.\n", argv[1]);

    return 0;
}
