// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	09/23/2017

// name: Lance Fletcher
// username of iLab: laf224
// iLab Server: composite.cs.rutgers.edu
#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE+

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include <ucontext.h>
#include <malloc.h>
#include <signal.h>
#include <sys/time.h>

//L: So our pthreads are just unsigned ints? I guess that means make a thread ID?
typedef uint my_pthread_t;

//L: struct to store thread attributes
typedef struct threadControlBlock
{
  my_pthread_t tid;
  my_pthread_t waitOn;
  unsigned int runTime = 0;
  unsigned int priority = 1;

  //0 = ready to run, 1 = yielding, 2 = waiting, 3 = mutex wait
  int status = 0;
  ucontext_t *context = NULL;
} tcb; 

/* mutex struct definition */
typedef struct my_pthread_mutex_t
{
  int locked;
  int available = 1;
  int holder = -1;
  list* queue;

} my_pthread_mutex_t;

/* define your data structures here: */

// Feel free to add your own auxiliary data structures

//L: Linked list 
typedef struct list
{
  tcb* thread = NULL;
  list* next = NULL;
}list;

/* Function Declarations: */

//L: queue functions
void enqueue(list**, tcb*);
tcb* dequeue(tcb**);

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#endif
