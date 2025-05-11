#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int main() {
    int inp1,inp2,answer1,i;
    scanf("%d %d",&inp1,&inp2); //& deya mane address ta diye deya
    if (inp1>=inp2){  //address er moddher value ta ante * deya lagbe
        for (i=1;i<=inp2;i++){
            if (inp1%i==0 && inp2%i==0){
                answer1=i;
            }
        }
    }
    else{
        for (i=1;i<=inp1;i++){
            if (inp1%i==0 && inp2%i==0){
                answer1=i;
            }
    }
    }
    printf("The GCD of %d and %d is %d\n", inp1, inp2, answer1);
    return 0;
}