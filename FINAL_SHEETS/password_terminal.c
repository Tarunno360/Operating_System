#include <stdio.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int a;
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    char pass[256];
    strcpy(pass,argv[1]);
    a=fork();
    if (a==0){
        //child process
        FILE *fp;
        fp=fopen('password.txt',"w");
        fprintf(fp, "%d", pass);
        fpclose(fp);
    }
    else{
        wait(NULL);
        
    }
    printf("Done writing to %s\n", argv[1]);
    return 0;
}