/*Driver Program to test pthread functions*/
#include <stdlib.h>
#include <stdio.h>
#include "my_pthread_t.h"

void* foo(void*);

int main(int argc, char* argv[])
{
  my_pthread_t t1, t2, t3, t4;

  //TODO: Create and test threads here
  printf("[Main] Hello World\n");

  int a = 1, b = 2, c = 3, d = 4;
  my_pthread_create(&t1, NULL, foo, (void*)&a);
  my_pthread_create(&t2, NULL, foo, (void*)&b);
  my_pthread_create(&t3, NULL, foo, (void*)&c); 
  my_pthread_create(&t4, NULL, foo, (void*)&d);

  printf("\nMain Exiting\n\n");

  my_pthread_exit(NULL);

  return 0;
}

void* foo(void* param)
{
  int i = *(int*)param;
  printf("\nHello From Thread %d\n\n", i);

  unsigned int n=1;
  for(n = 0; n < 0xffff; n++)
  {
    if(n%100000000==0)
    {
      printf("thread %i, n = %i", i, n);
    }
  }

  printf("\nThread %d exiting\n\n", i);

  my_pthread_exit(NULL);
  return NULL;
};
