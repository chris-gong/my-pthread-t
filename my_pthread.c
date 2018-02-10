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
#define EXIT 3
#define JOIN 4
#define MUTEX_WAIT 3
#define MAX_SIZE 60

tcb *currentThread = NULL, *prevThread = NULL;
list *runningQueue[MAX_SIZE];
list *waitingQueue;

struct itimerval timer;

int mainRetrieved = 0
int timeElapsed = 0;
int threadCount = 1;
int spliceTime;
//int pthreadJoinCalled = 0;

//TODO: create context dedicated to freeing finished threads

//L: Signal handler to reschedule upon VIRTUAL ALARM signal
void scheduler(int signum)
{
  //TODO: get the amount of time left and set it to currentThread->timeLeft
  //L: disable timer if still active
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
  setitimer(ITIMER_VIRTUAL, &timer, NULL);

  /*// retrieve first highest priority thread and set it to current thread
  // if pthread_join was not called, because in that case current thread will be set already
  if (!pthreadJoinCalled)
  {
    int i;
    for (i = 0; i < MAX_SIZE; i++) 
    {
      if (runningQueue[i] != NULL)
      {
        currentThread = dequeue(runningQueue[i]);
      }
    }
  }*/
  

  if(signum != SIGVTALRM)
  {
    /*TODO: PANIC*/
    printf("Signal Received: %d.\nExiting...\n", signum);
    exit(signum);
  }

  //L: check for maintenance cycle
  if(timeElapsed > 1000000) //L: NOTE: we could instead create a second handler for real-time alarm signal
  {
    //boost all the threads to the top priority
  }

  prevThread = currentThread;

  switch(currentThread->status)
  {
    case READY:
      //READY signifies that the current thread is in the running queue
      //either the currentThread is the main thread or its time splice ran out
      //and we're gonna retrieve the next highest priority thread from the running queue

      if(currentThread->priority < MAX_SIZE)
      {
	currentThread->priority++;
      }

      enqueue(runningQueue[currentThread->priority], currentThread);

      int i;
      for (i = 0; i < MAX_SIZE; i++) 
      {
        if (runningQueue[i] != NULL)
        { 
          currentThread = dequeue(runningQueue[i]);
	  break;
        }
      }

      break;
   
    case YIELD:
      //YIELD signifies pthread yield was called

      int i;
      for (i = 0; i < MAX_SIZE; i++) 
      {
        if (runningQueue[i] != NULL)
        { 
          currentThread = dequeue(runningQueue[i]);
	  break;
        }
      }
      
      if(currentThread != NULL)
      {
	//later consider enqueuing it to the waiting queue instead
	//yield wont change the priority if we are not sacrificing time splice
	enqueue(runningQueue[prevThread->priority], prevThread);
      }
      else
      {
	currentThread = prevThread;
      }

      break;

    case WAIT:
      //TODO: When would something go to waiting queue?
      //A: In the case of blocking I/O, how do we detect this? Sockets
      enqueue(waitingQueue[currentThread->priority-1], currentThread);
      
      break;
    case EXIT:
      //EXIT corresponds with a call to pthread_exit
      
      //free the thread control block and ucontext
      free(currentThread->context->uc_stack.ss_sp); //TODO: Make sure shit doesn't break here
      free(currentThread->context);
      free(currentThread);
      int i;
      for (i = 0; i < MAX_SIZE; i++) 
      {
        if (runningQueue[i] != NULL)
        { 
          currentThread = dequeue(runningQueue[i]);
	  break;
        }
      }
      currentThread->status = READY;
      currentThread->runTime = 0;  

      //TODO: set timeleft based on priority level, for now it's fixed
      //spliceTime = 25000;
   
      //L: reset timer to 25ms
      timer.it_value.tv_sec = 0;
      timer.it_value.tv_usec = currentThread->timeLeft;
      timer.it_interval.tv_sec = 0;
      timer.it_interval.tv_usec = 0;
      setitimer(ITIMER_VIRTUAL, &timer, NULL);
      setcontext(currentThread);
    case JOIN:
      
      
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

  //TODO: set timeleft based on priority level, for now it's fixed
  //spliceTime = 25000;

  //L: reset timer to 25ms
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = currentThread->timeLeft;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
  setitimer(ITIMER_VIRTUAL, &timer, NULL);

  //Switch to new context
  if(prevThread->tid == currentThread->tid)  
  {/*Assume switching to same context is bad. So don't do it.*/}
  else
  {swapcontext(prevThread->context, currentThread->context);}

  return;
}

//L: add to queue
void enqueue(list* queue, tcb* insert)
{
  if(queue == NULL)
  {
    queue = (list*)malloc(sizeof(list));
    queue->thread = insert;
    queue->next = queue;
    return;
  }

  //L: queue = last
  list* temp = queue->next;
  queue->next = (list*)malloc(sizeof(list));
  queue->next->thread = insert;
  queue->next->next = temp;
  queue = queue->next;
  return;
}

//L: add to queue
//L: TODO: rewrite to support list*
tcb* dequeue(list* queue)
{
  //queue[ is the last element in a queue at level i
  list* temp = queue->next;
  queue->next = temp->next;

  return temp->thread;
}

void jqInsert(list* queue)
{
  //TODO: for the join queue, make the linked list not circular
  //because otherwise we will have an infinite loop when freeing it


}

tcb* search(my_pthread_t)
{



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
  newThread->timeLeft = 25000;
  //TODO: newThread->tid = some unique number
  newThread->tid = threadCount;
  threadCount++;

  enqueue(runningQueue[0], newThread);

  //TODO: store main context
  if (!mainRetrieved)
  {
    tcb *mainThread = (tcb*)malloc(sizeof(tcb));
    mainThread->tid = 0;
    mainThread->timeLeft = 25000;
    ucontext_t *mText = (ucontext_t*)malloc(sizeof(ucontext_t));
    getcontext(mText);
    mainThread->context = mText;
    mainRetrieved = 1;
    initializeQueues(runningQueue, waitingQueue); //set everything to NULL

    //enqueue(runningQueue[0], mainThread);
    currentThread = mainThread;

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
  //dequeue all threads waiting on this one to finish
  list *removeNode = currentThread->joinQueue;
  list *currNode = removeNode
  tcb *jThread;
  jqThread->retVal = value_ptr;
  while(currNode != NULL)
  {
    jqThread = currNode->thread;
    enqueue(runningQueue[jqThread->priority], jqThread);
    currNode = currNode->next;
    free(removeNode);
    removeNode = currNode;
  }
  
  //L: free context stack, remove context from tcb, free control block
  /*free(currentThread->context->uc_stack.ss_sp); //TODO: Make sure shit doesn't break here
  free(currentThread->context);
  free(currentThread);*/

  //L: TODO: how do we transfer return value to thread join?

  //L: return to signal handler/scheduler
  raise(SIGVTALRM);
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr)
{
  //L: TODO: find way to search queues for a thread given its tid
  //L: once thread found, give thread a queue???

  //L: make sure thread can't wait on self
  if(thread == currentThread->tid)
  {return -1;}
  
  



  raise(SIGVTALRM);
  *value_ptr = currentThread->retVal;

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

void initializeQueues(list** runningQueue, list* waitingQueue) 
{
  waitingQueue = NULL;
  int i;
  for(i = 0; i < MAX_SIZE; i++) 
  {
    runningQueue[i] = NULL;
  }
  
}

