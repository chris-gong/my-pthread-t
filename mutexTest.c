/*Driver Program to test pthread functions*/
#include <stdlib.h>
#include <stdio.h>
#include "my_pthread_t.h"

void* voo(void*);
void* doo(void*);

my_pthread_t t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
my_pthread_mutex_t mutex;

int main(int argc, char* argv[])
{

  //TODO: Create and test threads here
  printf("[Main] Hello World\n");

  //my_pthread_mutex_init(&mutex, NULL);
  my_pthread_mutex_lock(&mutex);
  /*int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
  my_pthread_create(&t1, NULL, voo, (void*)&a);
  my_pthread_create(&t2, NULL, voo, (void*)&b);
  my_pthread_create(&t3, NULL, voo, (void*)&c); 
  my_pthread_create(&t4, NULL, voo, (void*)&d);
  my_pthread_create(&t5, NULL, doo, (void*)&e);
  my_pthread_create(&t6, NULL, doo, (void*)&f);
  my_pthread_create(&t7, NULL, voo, (void*)&g);
  my_pthread_create(&t8, NULL, voo, (void*)&h);
  my_pthread_create(&t9, NULL, voo, (void*)&i);
  my_pthread_create(&t10, NULL, voo, (void*)&j);*/
  my_pthread_yield();
  void *ret;
  my_pthread_join(0, &ret);
  printf("\nMain Exiting\n\n");
  //my_pthread_mutex_unlock(&mutex);
  my_pthread_mutex_destroy(&mutex);
  my_pthread_exit(NULL);
  
  return 0;
}

void* voo(void* param)
{
  int i = *(int*)param;

  my_pthread_mutex_lock(&mutex);
  printf("\n[%d] LOCK ACQUIRED\n\n", i);

  printf("\nHello From Thread %d\n\n", i);

  

  unsigned int n=1;
  for(n = 0; n < 0xffffff; n++)
  {
    
  }


  my_pthread_mutex_unlock(&mutex);
  printf("\n[%d] LOCK RELEASED\n\n", i);
  printf("\nThread %d exiting\n lockholder:%i\n", i, mutex.holder);

 
  my_pthread_exit(NULL);
  return NULL;
};

void *doo(void* param)
{
  int i = *(int*)param;
  printf("\nHello From Thread %d\n\n", i);

  unsigned int n=0;
  for(n = 0; n < 0xffffffff; n++)
  {
    //L: spin
  }

  if(i == 5)
  {
    printf("\n MUTEX LOCK QUEUE IS NULL \n");
    my_pthread_mutex_destroy(&mutex);
  }

  printf("\nThread %d exiting\n\n", i);

  my_pthread_exit(NULL);
  return NULL;
}
