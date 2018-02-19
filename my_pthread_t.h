// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	09/23/2017

// name: Lance Fletcher, Jeremy Banks (jpb231), Christopher Gong
// username of iLab: laf224
// iLab Server: composite.cs.rutgers.edu
#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define pthread_create(a, b, c, d) my_pthread_create(a, b, c, d)
#define pthread_yield() my_pthread_yield()
#define pthread_exit(a) my_pthread_exit(a)
#define pthread_join(a, b) my_pthread_join(a, b)
#define pthread_mutex_init(a, b) my_pthread_mutex_init(a, b)
#define pthread_mutex_lock(a) my_pthread_mutex_lock(a)
#define pthread_mutex_unlock(a) my_pthread_mutex_unlock(a)
#define pthread_mutex_destroy(a) my_pthread_mutex_destroy(a)

#define _GNU_SOURCE

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <malloc.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

//L: So our pthreads are just unsigned ints? I guess that means make a thread ID?
typedef uint my_pthread_t;
#define pthread_t my_pthread_t

typedef struct sigaction mySig;

//L: struct to store thread attributes
typedef struct threadControlBlock
{
  my_pthread_t tid;
  unsigned int priority;

  //0 = ready to run, 1 = yielding, 2 = waiting, 3 = exiting, 4 = joining, 5 = waiting for mutex lock
  int status;
  void* jVal;
  void* retVal;
  ucontext_t *context;
  struct list* joinQueue;
} tcb;

//L: Linked list 
typedef struct list
{
  tcb* thread;
  struct list* next;
}list;

/* mutex struct definition */
typedef struct my_pthread_mutex_t
{
  int locked;
  int available;
  int holder;
  int initialized;
  list* queue;

} my_pthread_mutex_t;
#define pthread_mutex_t my_pthread_mutex_t

/* define your data structures here: */

// Feel free to add your own auxiliary data structures

/* Function Declarations: */

//L: queue functions
void enqueue(list**, tcb*);
tcb* dequeue(list**);

//L: linked list functions
void l_insert(list**, tcb*);
tcb* l_remove(list**);

//L: table functions
tcb* thread_search(my_pthread_t);

//L: init functions
void initializeQueues(list**);

//L: maintenance: boost thread priorities
void maintenance();

//L: free threads that don't exit properly
void garbage_collection();

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

void initializeMainContext();

void initializeGarbageContext();
#endif

