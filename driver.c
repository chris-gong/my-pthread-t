/*Driver Program to test pthread functions*/
#include <stdlib.h>
#include <stdio.h>
#include "my_pthread_t.h"

void* foo(void*);

int main(int argc, char* argv[])
{
	my_pthread_t t1, t2, t3;

	//TODO: Create and test threads here
	printf("Hello World\n");

	int n = 1;

	my_pthread_create(&t1, NULL, foo, (void*)&n);
	n++;
	my_pthread_create(&t2, NULL, foo, (void*)&n);
	n++;
	my_pthread_create(&t3, NULL, foo, (void*)&n);

	printf("Checkpoint\n");

	my_pthread_exit(NULL);

	return 0;
}

void* foo(void* param)
{
	int i = *(int*)param;
	printf("Hello From %d\n", i);

	my_pthread_exit(NULL);
	return NULL;
};
