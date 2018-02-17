/*Driver Program to test pthread functions*/
#include <stdlib.h>
#include <stdio.h>
#include "my_pthread_t.h"

void* voo(void*);
void* doo(void*);

my_pthread_t t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
int someNum;

int main(int argc, char* argv[])
{
  //TODO: Create and test threads here
  printf("[Main] Hello World\n");
  //int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
  /*my_pthread_create(&t1, NULL, doo, (void*)&a);
  my_pthread_create(&t2, NULL, voo, (void*)&b);
  my_pthread_create(&t3, NULL, voo, (void*)&c); 
  my_pthread_create(&t4, NULL, voo, (void*)&d);
  my_pthread_create(&t5, NULL, voo, (void*)&e);
  void *mainRet;
  my_pthread_join(t1, &mainRet);
  my_pthread_create(&t6, NULL, voo, (void*)&f);
  my_pthread_create(&t7, NULL, voo, (void*)&g);
  my_pthread_create(&t8, NULL, voo, (void*)&h);
  my_pthread_create(&t9, NULL, voo, (void*)&i);
  my_pthread_create(&t10, NULL, voo, (void*)&j);

  printf("\nMain Exiting\n\n");*/
  my_pthread_yield();
  my_pthread_exit(NULL);

  return 0;
}

void* voo(void* param)
{
  int i = *(int*)param;
  void *ret;
  printf("\nHello From Thread %d\n\n", i);
  int code = 0;
  printf("\nAfter yield From Thread %d\n\n", i);
  if(my_pthread_join(t1, &ret) != -1)
  {
    code = *(int*)ret;
  }
  unsigned int n=1;
  for(n = 0; n < 0xffffffff; n++)
  {

  }

  printf("\nAfter yield and loop From Thread %d\n\n", i);

  printf("someNum from thread 1: %i \n", code);
  printf("\nThread %d exiting with code\n\n", i);

  my_pthread_exit(NULL);
  return NULL;
};

void *doo(void* param)
{
  int i = *(int*)param;
  printf("\nHello From Thread %d\n\n", i);
  printf("\nAfter yield From Thread %d\n\n", i);
  unsigned int n=0;
  for(n = 0; n < 0xffffffff; n++)
  {
    //L: spin
  }
  printf("\nAfter yield and loop From Thread %d\n\n", i);
  someNum = 5;

  printf("\nThread %d exiting\n\n", i);

  my_pthread_exit(&someNum);
  return NULL;
}
