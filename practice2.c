#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
static int a=5;
static int b=3;
int main(){
pid_t x, y;
x=fork();
if(x<0){
printf("fork failed\n");
}
else if(x>0){
a=a+5;
b=b-5;
wait(NULL);
y=fork();
if(y<0){
printf("fork failed\n");
}
else if(y>0){
wait(NULL);
a=a-2;
b=b+2;
}
else{
a=a*2;
b=b/3;
}
}
else{
a=a/2;
b=b*3;
}
printf("a= %d\n",a);
printf("b= %d\n",b);
return 0;


}