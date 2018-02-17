/*Driver Program to test pthread functions*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

void* voo(void*);
void* doo(void*);

pthread_t t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
int someNum;

int main(int argc, char* argv[])
{
  //TODO: Create and test threads here
  printf("[Main] Hello World\n");
  int sharedResource = 0;
  pthread_mutex_t mutex;
  pthread_mutex_lock(&mutex);
  int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
  pthread_create(&t1, NULL, doo, (void*)&a);
  pthread_create(&t2, NULL, voo, (void*)&b);
  pthread_create(&t3, NULL, voo, (void*)&c); 
  pthread_create(&t4, NULL, voo, (void*)&d);
  pthread_create(&t5, NULL, voo, (void*)&e);
  pthread_create(&t6, NULL, voo, (void*)&f);
  pthread_create(&t7, NULL, voo, (void*)&g);
  pthread_create(&t8, NULL, voo, (void*)&h);
  pthread_create(&t9, NULL, voo, (void*)&i);
  pthread_create(&t10, NULL, voo, (void*)&j);

  printf("\nMain Exiting\n\n");

  pthread_exit(NULL);

  return 0;
}

void* voo(void* param)
{
  int i = *(int*)param;
  void *ret;
  printf("\nHello From Thread %d\n\n", i);

  unsigned int n=1;
  for(n = 0; n < 0xffffffff; n++)
  {
    if(n % 2543 == 0)
    {
      //printf("thread %i on n = %i\n", i, n);
    }
  }

  pthread_join(t1, &ret);
  //int code = *(int*)ret;

  printf("\nThread %d exiting with code\n\n", i);

  pthread_exit(NULL);
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

  someNum = 5;

  printf("\nThread %d exiting\n\n", i);

  pthread_exit(&someNum);
  return NULL;
}
