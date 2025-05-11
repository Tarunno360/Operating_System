#include <stdio.h>

int main()
{
	int num1,num2;

	char str[100];
	int smaller;
	int result;


	printf("Enter two numbers: ");
	scanf("%d, %d", &num1, &num2);
	if(num1 < num2){
		smaller = num1;
	}
	else{
		smaller = num2;
	}

	for(int i = 1; i <= smaller; i++){
		if(num1%i == 0 && num2%i == 0){
			result = i;
		} 
	}
	printf("GCD is: %d\n", result);

    return 0;
}

