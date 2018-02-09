// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: Lance Fletcher
// username of iLab: laf224
// iLab Server: composite.cs.rutgers.edu

#include "my_pthread_t.h"

#define STACK_S 4096 //4kB stack frames
#define READY 0
#define YIELD 1
#define WAIT 2
#define MUTEX_WAIT 3

tcb *currentThread;
list** runningQueue;
list** waitingQueue;

struct itimerval timer;

int init = 0, elapsed = 0, size = 0;
const int MAX_SIZE 60;

//L: Signal handler to reschedule upon VIRTUAL ALARM signal
void scheduler(int signum)
{
  //L: disable timer if still active
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
  setitimer(ITIMER_VIRTUAL, &timer, NULL);

  if(signum != SIGVTALRM)
  {
    /*TODO: PANIC*/
    printf("Signal Received: %d.\nExiting...\n", signum);
    exit(signum);
  }

  tcb *prevThread = NULL;

  //L: Update cycle count
  elapsed++;

  //L: check for maintenance cycle
  if(elapsed % 4 == 0) //L: NOTE: we could instead create a second handler for real-time alarm signal
  {
    /*maintenance();*/
  }

  //TODO: implement this
  switch(currentThread->status)
  {
    case READY:
      currentThread->runTime ++;
      if(currentThread->priority > currentThread->runTime) //L: NOTE: assuming we're sticking with a linear progression for priority:runtime ratio
      {break;}

      if(currentThread->priority < MAX_SIZE)
      {currentThread->priority ++;}

      enqueue(runningQueue[currentThread->priority-1], currentThread);
      prevThread = currentThread;
      currentThread = dequeue(runningQueue);

      break;
   
    case YIELD:
      //L: Don't alter priority, dequeue before enqeueuing current thread
      prevThread = currentThread;
      currentThread = dequeue(runningQueue);
      
      if(currentThread != NULL)
      {enqueue(runningQueue[prevThread->priority-1], prevThread);}
      else
      {currentThread = prevThread;}

      break;

    case WAIT:
      //TODO: When would something go to waiting queue?
      enqueue(waitingQueue[currentThread->priority-1], currentThread);
      
      break;

    case MUTEX_WAIT:
      //L: Don't add current to queue: already in mutex queue
      prevThread = currentThread;
      currentThread = dequeue(runningQueue);
      break;

    default:
      printf("Thread Status Error: %d\n", currentThread->status);
      exit(-1);
      break;
  }
	
  currentThread->status = READY;
  currentThread->runTime = 0;  

  //L: reset timer to 25ms
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 25000;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 25000;
  setitimer(ITIMER_VIRTUAL, &timer, NULL);

  //Switch to new context
  if(prevThread->tid == currentThread->tid)  
  {/*Assume switching to same context is bad. So don't do it.*/}
  else
  {swapcontext(prevThread->context, currentThread->context);}

  return;
}

//L: add to queue
//L: TODO: rewrite so 1st param is list*
void enqueue(list** queue, tcb* insert)
{
  int key = insert->priority-1;

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

//L: add to queue
//L: TODO: rewrite to support list*
tcb* dequeue(list** queue)
{
  //queue[i] is the last element in a queue at level i

  int i;
  int len = sizeof(queue)/sizeof(list*); //L: NOTE: this may not work please check
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
  signal(SIGVTALRM, scheduler);

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
  //TODO: newThread->tid = some unique number
  newThread->tid = size;
  size++;

  enqueue(runningQueue, newThread);

  if(!init)
  {
    init = 1;
    raise(SIGVTALRM);
  }
  return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield()
{
  //L: return to signal handler/scheduler
  currentThread->status = YIELD;
  return raise(SIGVTALRM);
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr)
{
  //TODO: dequeue all threads waiting on this one to finish

  //L: free context stack, remove context from tcb, free control block
  free(currentThread->context->uc_stack.ss_sp); //TODO: Make sure shit doesn't break here
  free(currentThread->context);
  free(currentThread);

  //L: TODO: how do we transfer return value to thread join?

  //L: return to signal handler/scheduler
  raise(SIGVTALRM);
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr)
{
  //L: TODO: find way to search queues for a thread given its tid
  //L: once thread found, give thread a queue???
  return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
  //L: allocate new mutex struct
  mutex = (my_pthread_mutex_t*)malloc(sizeof(my_pthread_mutex_t));
  mutex->locked = 0;
  mutex->queue = NULL;
  return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex)
{
  //L: TODO: check if mutex has been initialized?

  //L: loop to mitigate race condition
  while(mutex->locked)
  {
    enqueue(mutex->queue, currentThread); //L: NOTE: this will not work until enqueue has been rewritten
    currentThread->status = MUTEX_WAIT;
    raise(SIGVTALRM);
  }

  if(!mutex->available)
  {return -1;}

  mutex->locked = 1;
  mutex->holder = currentThread->tid;

  return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex)
{
  //NOTE: handle errors: unlocking an open mutex, unlocking mutex not owned by calling thread, or accessing unitialized mutex
  //L: TODO: feel free to change return values

  if(!mutex->locked)
  {return -1;}
  
  else if(mutex->holder != currentThread->tid)
  {return -2;}

  mutex->locked = 0;
  mutex->holder = -1;

  tcb* muThread = dequeue(mutex->queue); //L: NOTE: this will not work until dequeue has been rewritten

  if(muThread != NULL)
  {enqueue(runningQueue[muThread->priority], muThread);} //L: NOTE: won't work until enqueue rewrite

  return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex)
{
  //L: prevent threads from accessing while mutex is in destruction process
  mutex->available = 0;
  
  //L: if mutex still locked, wait for thread to release lock
  while(mutex->locked)
  {raise(SIGVTALRM);}

  //L: TODO: Is implementing a free-for-all post-destruction a terrible idea? This requires research...
  list *temp;
  tcb *muThread;
  while(mutex->queue != NULL)
  {
    temp = mutex->queue;
    muThread = dequeue(mutex->queue);
    enqueue(runningQueue[mThread->priority], mThread);
    mutex->queue = mutex->queue->next;
    free(temp);
  }

  return 0;
};

