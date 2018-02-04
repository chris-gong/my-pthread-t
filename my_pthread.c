// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: Lance Fletcher
// username of iLab: laf224
// iLab Server: composite.cs.rutgers.edu

#include "my_pthread_t.h"

#define STACK_S 4096 //4kB stack frames

tcb *currentThread;
list* runningQueue[10];
list* waitingQueue[10];

struct itimerval timer;
timer.it_value.tv_sec = 0;
timer.it_value.tv_usec = 25000;
timer.it_interval.tv_sec = 0;
timer.it_interval.tv_usec = 25000;

int init = 0, elapsed = 0, size;


//L: Signal handler to reschedule upon ALARM signal
void signal_handler(int signum)
{
  if(signum != SIGALRM)
  {/*TODO: PANIC*/}	
	
  //Update cycle num
  elapsed++;

  //Update heaps and reset alarm timer
  currentThread->runTime ++;

  //TODO: update based on thread priority and reallocate
  //if(currentThread->priority...)

  //L: check for maintenance cycle
  //L: this might not be exact || if(timer.it_value.tv_usec % (suseconds_t)100000 == 0)
  if(elapsed % 4 == 0)
  {
    /*maintenance();*/
  }

  //L: reset timer to 25ms
  setitimer(ITIMER_VIRTUAL, &timer, NULL);
  setcontext(/*dequeue()*/);	
}

//L: TODO: heap functions

void enqueue(list** queue, tcb* insert)
{
  int key = insert->priority;

  if(queue[key] == NULL)
  {
    queue[key] = (list*)malloc(sizeof(list));
    queue[key]->thread = insert;
    queue[key]->next = queue[key];
    return;
  }

  //L: queue[key] = last
  list* temp = queue[key]->next;
  queue[key]->next = (list*)malloc(sizeof(list));
  queue[key]->next->thread = insert;
  queue[key]->next->next = temp;
  queue[key] = queue[key]->next;

}

tcb* dequeue(list** queue)
{
  //queue[i] is the last element in a queue at level i

  int i;
  int len = sizeof(queue)/sizeof(list*); //TODO: note, this may not work please check
  for(i = 0; i < len; i++)
  {
    //look for the highest priority level with at least one thread block
    if(queue[i] != NULL)
    {
      break;
    }
  }
  if(i >= len)
  {
    //multi-level queue is empty at every level
  }
  
  list* temp = queue[i]->next;
  queue[i]->next = temp->next;

  return temp->thread;
}

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg)
{
  int sig = signal(SIGALRM, signal_handler);

  //L: Create a thread context to add to scheduler
  ucontext_t* task = (ucontext_t*)malloc(sizeof(ucontext_t));
  getcontext(task);
  task->uc_link = NULL;
  task->uc_stack.ss_sp = malloc(STACK_S);
  task->uc_stack.ss_size = STACK_S;
  task->uc_stack.ss_flags = 0;
  makecontext(&task, function, 1, arg);

  tcb *newThread = (tcb*)malloc(sizeof(tcb));
  newThread->context = task;

  //TODO: enqueue(runningQueue, newThread);

  if(!init)
  {
    init = 1;
    //setitimer(ITIMER_VIRTUAL, &boostTimer, NULL);
    raise(SIGALRM);
  }
  return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield()
{
  //L: return to signal handler/scheduler
  return raise(SIGALRM);
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr)
{
  //L: free context stack, remove context from tcb, free control block
  free(currentThread->context->uc_stack.ss_sp);
  free(currentThread->context);
  free(currentThread);
  //TODO: Make sure shit doesn't break here
  //L: return to signal handler/scheduler
  raise(SIGALRM);
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr)
{
  return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
  //
  return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex)
{
  return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex)
{
  return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex)
{
  return 0;
};

