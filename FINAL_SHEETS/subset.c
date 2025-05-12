#include <stdio.h>
#include <stdbool.h>
int main()
{
	int arr1Length,arr2Length;

	char str[100];
	int elem;
	bool is_subset = true;


	printf("Enter the length of array 1: ");
	scanf("%d", &arr1Length);
	int arr1[arr1Length];
	
	printf("Enter the elements of array 1: ");
	for(int i = 0; i < arr1Length; i++){
		scanf("%d", &elem);
		arr1[i] = elem;
		
	}
	
	
	printf("Enter the length of array 2: ");
	scanf("%d", &arr2Length);
	int arr2[arr2Length];
	bool contains[arr2Length];
	printf("Enter the elements of array 2: ");
	for(int i = 0; i < arr2Length; i++){
		scanf("%d", &elem);
		for(int j = 0; j < arr1Length; j++){
			if (arr1[j] == elem){
				contains[i] = true;
			} 
		}
		arr2[i] = elem;
		
	}
	
	for (int i = 0; i < arr2Length; i++){
		if (contains[i] == false){
			is_subset = false;
		}
	}
	if(is_subset){
		printf("Arr2 subset of Arr1");
	}
	else{
		printf("Arr2 not subset of Arr1");
	}
			
	
	
    return 0;
}