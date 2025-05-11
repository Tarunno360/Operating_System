#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
void main()
{	FILE *fp;

	int a, status;


	a = fork();
	char buff[256];

	if(a >0){
		wait(&status);
		printf("CSE321.txt opened for read access\n");
		fp = fopen("a.txt", "r");
		
		printf("CSE321.txt contains....\n");
		while (fgets(buff, sizeof(buff), fp)) {
            		printf("%s\n", buff);  
            	}
		fclose(fp);
		printf("CSE321.txt closed for read access\n");
		
	}	
	else if(a==0){
		printf("CSE321.txt opened for write access\n");
		fp = fopen("a.txt", "w");
		fprintf(fp, "aaaaa\nbbbbb\nccccc");
		fclose(fp);
		printf("CSE321.txt closed for write access\n");
	}
	
}


