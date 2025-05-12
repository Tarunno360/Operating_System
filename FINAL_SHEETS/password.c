#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
int main(int argc, char *argv[])
{	char buff[256];
	FILE *fp;	
	//char password[256];
	//strcpy(password, argv[1]);
	
	char *password = argv[1];
	
	int a, status;
	a = fork();
	if(a==0){
		
		fp = fopen("password.txt", "w");
		fprintf(fp, "%s", password);

		fclose(fp);
	}
	
	else if(a>0){
		wait(&status);
		fp = fopen("password.txt", "r");
		fgets(buff, sizeof(buff), fp);
		fclose(fp);
		 
		for(int i = 0; buff[i] != '\0'; i++ ){
			char ch = tolower(buff[i]);
			if (ch == 'a' || ch== 'b' || ch== 'c' || ch == 'd') {
                printf("Yes\n");
            }
			
			 
		}
		return 0;
		  	
	}
	}
	
	



