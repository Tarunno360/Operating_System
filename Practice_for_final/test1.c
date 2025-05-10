#include <stdio.h>

int main(){
    int x,y;
    char str[50];
    FILE *fp;
    fp=fopen("in.txt","r");
    fgets(str,50,fp);
    printf("%s\n",str);
    fclose(fp);
}
