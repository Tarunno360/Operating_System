#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
int main(){
    pid_t p;
    int a=3;
    int b=11;
    char s[20];
    p=fork();
    if(p<0){
    printf("fork failed\n");
    }
    else if(p==0){
    strcpy(s,"child");
    a=a*b;
    b=b/a;
    
    }
    else{
    wait(NULL); 
    strcpy(s,"parent");
    a=a+b;
    b=b-a;
    
    }
    printf("%s is printing a= %d\n",s,a);
    printf("%s is printing b= %d\n",s,b);
    return 0;
    }