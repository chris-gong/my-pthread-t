/*Driver Program to test pthread functions*/
#include <stdlib.h>
#include <stdio.h>
#include "my_pthread_t.h"

void* foo(void*);

int main(int argc, char* argv[])
{
  my_pthread_t t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;

  //TODO: Create and test threads here
  printf("[Main] Hello World\n");

  int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
  my_pthread_create(&t1, NULL, foo, (void*)&a);
  my_pthread_create(&t2, NULL, foo, (void*)&b);
  my_pthread_create(&t3, NULL, foo, (void*)&c); 
  my_pthread_create(&t4, NULL, foo, (void*)&d);
  my_pthread_create(&t5, NULL, foo, (void*)&e);
  my_pthread_create(&t6, NULL, foo, (void*)&f);
  my_pthread_create(&t7, NULL, foo, (void*)&g);
  my_pthread_create(&t8, NULL, foo, (void*)&h);
  my_pthread_create(&t9, NULL, foo, (void*)&i);
  my_pthread_create(&t10, NULL, foo, (void*)&j);

  printf("\nMain Exiting\n\n");

  my_pthread_exit(NULL);

  return 0;
}

void* foo(void* param)
{
  int i = *(int*)param;
  printf("\nHello From Thread %d\n\n", i);

  unsigned int n=1;
  for(n = 0; n < 0xffffffff; n++)
  {
    /*
    if(n%100000000==0)
    {
      printf("thread %i, n = %i", i, n);
    }
    */
  }

  printf("\nThread %d exiting\n\n", i);

  my_pthread_exit(NULL);
  return NULL;
};
