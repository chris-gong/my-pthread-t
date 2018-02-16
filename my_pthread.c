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
//TODO: combine JOIN and MUTEX cases; they're handled the same way
#define JOIN 4
#define MUTEX_WAIT 5
#define MAX_SIZE 60
#define INTERVAL 25000

tcb *currentThread, *prevThread;
list *runningQueue[MAX_SIZE];
list *allThreads[MAX_SIZE];
ucontext_t cleanup;
sigset_t signal_set;

struct itimerval timer, currentTime;

int mainRetrieved;
int timeElapsed;
int threadCount;


//L: Signal handler to reschedule upon VIRTUAL ALARM signal
void scheduler(int signum)
{
  //printf("Signaled from %d\n", currentThread->tid);
  //Record remaining time
  getitimer(ITIMER_VIRTUAL, &currentTime);

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

  int t;

  //L: Time elapsed = difference between max interval size and time remaining in timer
  if(INTERVAL * (currentThread->priority + 1) > (int)currentTime.it_value.tv_usec)
  {
    timeElapsed += ((INTERVAL * (currentThread->priority + 1)) - (int)currentTime.it_value.tv_usec);
    t = (int)currentTime.it_value.tv_usec;
  }

  else
  {
    timeElapsed += (INTERVAL * (currentThread->priority + 1));
    t = INTERVAL * (currentThread->priority + 1);
  }

  printf("Total time: %d from time remaining: %d out of %d\n", timeElapsed, (int)currentTime.it_value.tv_usec, INTERVAL * (currentThread->priority + 1));

  //L: check for maintenance cycle
  if(timeElapsed >= 100000)
  {
    printf("\nMAINTENANCE TRIGGERED\n\n");
    maintenance();

    //L: reset counter
    timeElapsed = 0;
  }

  prevThread = currentThread;
  
  int i;

  switch(currentThread->status)
  {
    case READY: //READY signifies that the current thread is in the running queue
      //printf("%d reports READY\n", currentThread->tid);
      if(currentThread->priority < MAX_SIZE)
      {
	currentThread->priority++;
      }

      //printf("%d enqueued\n", currentThread->tid);
      enqueue(&runningQueue[currentThread->priority], currentThread);

      currentThread = NULL;

      for(i = 0; i < MAX_SIZE; i++) 
      {
        if (runningQueue[i] != NULL)
        { 
          currentThread = dequeue(&runningQueue[i]);
	  //printf("Dequeuing thread ID %d\n", currentThread->tid);
	  break;
        }
	else
	{
	  //printf("What #%d\n", i);
	}
      }

      if(currentThread == NULL)
      {
        currentThread = prevThread;
      }

      break;
   
    case YIELD: //YIELD signifies pthread yield was called; don't update priority
      //printf("%d reports YIELD\n", currentThread->tid);
      currentThread = NULL;

      for (i = 0; i < MAX_SIZE; i++) 
      {
        if (runningQueue[i] != NULL)
        { 
          currentThread = dequeue(&runningQueue[i]);
	  break;
        }
      }
      
      if(currentThread != NULL)
      {
	//later consider enqueuing it to the waiting queue instead
	enqueue(&runningQueue[prevThread->priority], prevThread);
      }
      else
      {
	currentThread = prevThread;
      }

      break;

    case WAIT:
      //L: When would something go to waiting queue?
      //A: In the case of blocking I/O, how do we detect this? Sockets
      //L: GG NOT OUR PROBLEM ANYMORE
      enqueue(&waitingQueue, currentThread);
      
      break;

    case EXIT:
      //printf("%d reports EXIT\n", currentThread->tid);
      prevThread = currentThread;
      currentThread = NULL;

      for (i = 0; i < MAX_SIZE; i++) 
      {
        if (runningQueue[i] != NULL)
        {
	  //printf("Searching... %d\n", runningQueue[i]->thread->tid);
          currentThread = dequeue(&runningQueue[i]);
          //printf("Queuing up %d\n", currentThread->tid);
	  break;
        }
      }

      //TODO: we may have a problem here
      if(currentThread == NULL)
      {
	//L: what if other threads exist but none are in running queue?
	printf("No other threads found. Exiting\n");
	exit(EXIT_SUCCESS);
      }

      //L: free the thread control block and ucontext
      free(prevThread->context->uc_stack.ss_sp);
      free(prevThread->context);
      free(prevThread);

      currentThread->status = READY;

      //L: reset timer
      timer.it_value.tv_sec = 0;
      timer.it_value.tv_usec = INTERVAL * (currentThread->priority + 1);
      timer.it_interval.tv_sec = 0;
      timer.it_interval.tv_usec = 0;
      setitimer(ITIMER_VIRTUAL, &timer, NULL);
      setcontext(currentThread->context);

      break;

    case JOIN: //JOIN corresponds with a call to pthread_join

      prevThread = currentThread;
      currentThread = NULL;

      for (i = 0; i < MAX_SIZE; i++) 
      {
        if (runningQueue[i] != NULL)
        { 
          currentThread = dequeue(&runningQueue[i]);
	  break;
        }
      }

      if(currentThread == NULL)
      {
	/*TODO: WE'VE GOT A PROBLEM*/
	exit(EXIT_FAILURE);
      }
      
      break;
      
    case MUTEX_WAIT: //MUTEX_WAIT corresponds with a thread waiting for a mutex lock

      //L: Don't add current to queue: already in mutex queue
      prevThread = currentThread;
      currentThread = NULL;

      for (i = 0; i < MAX_SIZE; i++) 
      {
        if (runningQueue[i] != NULL)
        { 
          currentThread = dequeue(&runningQueue[i]);
	  break;
        }
      }

      if(currentThread == NULL)
      {
        /*TODO: OH SHIT DEADLOCK*/
	exit(EXIT_FAILURE);
      }

      break;

    default:
      printf("Thread Status Error: %d\n", currentThread->status);
      exit(-1);
      break;
  }
	
  currentThread->status = READY;

  //L: reset timer to 25ms times thread priority
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = INTERVAL * (currentThread->priority + 1);
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
  setitimer(ITIMER_VIRTUAL, &timer, NULL);

  printf("Switching to: TID %d Priority %d\n", currentThread->tid, currentThread->priority);
  //Switch to new context
  if(prevThread->tid == currentThread->tid)  
  {/*Assume switching to same context is bad. So don't do it.*/}
  else
  {swapcontext(prevThread->context, currentThread->context);}

  return;
}

//L: thread priority boosting
void maintenance()
{
  int i;
  list *temp;
  for(i = 0; i < MAX_SIZE; i++)
  {
    temp = allThreads[i];

    while(allThreads[i] != NULL)
    {
      //L: for now, assume all threads are boosted to top
      allThreads[i]->thread->priority = 0;


      allThreads[i] = allThreads[i]->next;
    }
    allThreads[i] = temp;
  }

  return;
}

//L: handle exiting thread: supports invoked/non-invoked pthread_exit call
void garbage_collection()
{
  //L: Block signal here
  sigprocmask(SIG_BLOCK, &signal_set, NULL);
  
  if(!mainRetrieved)
  {
    exit(EXIT_SUCCESS);
  }

  tcb *jThread = NULL;

  //L: dequeue all threads waiting on this one to finish
  while(currentThread->joinQueue != NULL)
  {
    jThread = l_remove(&currentThread->joinQueue);
    jThread->retVal = currentThread->jVal;
    enqueue(&runningQueue[jThread->priority], jThread);
  }

  //L: free stored node in allThreads
  int key = currentThread->tid % MAX_SIZE;
  if(allThreads[key]->thread->tid == currentThread->tid)
  {
    list *removal = allThreads[key];
    allThreads[key] = allThreads[key]->next;
    free(removal); 
  }

  else
  {
    list *temp = allThreads[key];
    while(allThreads[key]->next != NULL)
    {
      if(allThreads[key]->next->thread->tid == currentThread->tid)
      {
	list *removal = allThreads[key]->next;
	allThreads[key]->next = removal->next;
	free(removal);
        break;
      }
      allThreads[key] = allThreads[key]->next;
    }

    allThreads[key] = temp;
  }

  sigprocmask(SIG_UNBLOCK, &signal_set, NULL);
  raise(SIGVTALRM);
}

//L: add to queue
void enqueue(list** q, tcb* insert)
{
  list *queue = *q;

  if(queue == NULL)
  {
    queue = (list*)malloc(sizeof(list));
    queue->thread = insert;
    queue->next = queue;
    *q = queue;
    //printf("%d inserted [NULL]\n", insert->tid);
    return;
  }

  list *front = queue->next;
  queue->next = (list*)malloc(sizeof(list));
  queue->next->thread = insert;
  queue->next->next = front;

  queue = queue->next;
  *q = queue;
  //printf("%d inserted [NOT NULL]\n", insert->tid);
  return;
}

//L: remove from queue
tcb* dequeue(list** q)
{
  list *queue = *q;

  //queue is the last element in a queue at level i
  list *front = queue->next;
  tcb *tgt = queue->next->thread;
  queue->next = front->next;
  free(front);

  if(queue->next == queue)
  {
    queue = NULL;
  }

  
  if(tgt == NULL)
  {printf("WE HAVE A PROBLEM IN DEQUEUE\n");}

  *q = queue;
  return tgt;
}

//L: insert to list
void l_insert(list** q, tcb* jThread) //Non-circular Linked List
{
  list *queue = *q;

  if(queue == NULL)
  {
    queue = (list*)malloc(sizeof(list));
    queue->thread = jThread;
    *q = queue;
    return;
  }

  list *newNode = (list*)malloc(sizeof(list));
  newNode->thread = jThread;

  //L: append to front of LL
  newNode->next = queue;
  
  queue = newNode;
  *q = queue;
  return;
}

//L: remove from list
tcb* l_remove(list** q)
{
  list *queue = *q;

  if(queue == NULL)
  {
    return NULL;
  }

  list *temp = queue;
  tcb *ret = queue->thread;
  queue = queue->next;
  free(temp);
  *q = queue;
  return ret;
}


//L: Search table for a tcb given a uint
tcb* thread_search(my_pthread_t tid)
{
  int key = tid % MAX_SIZE;
  tcb *ret = NULL;

  list *temp = allThreads[key];
  while(allThreads[key] != NULL)
  {
    if(allThreads[key]->thread->tid == tid)
    {
      ret = allThreads[key]->thread;
      break;
    }
    allThreads[key] = allThreads[key]->next;
  }

  allThreads[key] = temp;

  return ret;
}

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg)
{
  signal(SIGVTALRM, scheduler);
  //TODO: L: I forget; why is it bad to intialize main before initializing new thread?

  if(!mainRetrieved)
  {
    initializeQueues(runningQueue); //set everything to NULL

    //Initialize garbage collector
    getcontext(&cleanup);
    cleanup.uc_link = NULL;
    cleanup.uc_stack.ss_sp = malloc(STACK_S);
    cleanup.uc_stack.ss_size = STACK_S;
    cleanup.uc_stack.ss_flags = 0;
    makecontext(&cleanup, (void*)&garbage_collection, 0);

    //L: intialize signal blocker
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGVTALRM);

    //L: set thread count
    threadCount = 1;
  }

  //L: Create a thread context to add to scheduler
  ucontext_t* task = (ucontext_t*)malloc(sizeof(ucontext_t));
  getcontext(task);
  task->uc_link = &cleanup;
  task->uc_stack.ss_sp = malloc(STACK_S);
  task->uc_stack.ss_size = STACK_S;
  task->uc_stack.ss_flags = 0;
  makecontext(task, (void*)function, 1, arg);

  tcb *newThread = (tcb*)malloc(sizeof(tcb));
  newThread->context = task;
  newThread->tid = threadCount;
  newThread->priority = 0;
  newThread->joinQueue = NULL;
  newThread->jVal = NULL;
  newThread->retVal = NULL;
  newThread->status = READY;

  *thread = threadCount;
  threadCount++;

  enqueue(&runningQueue[0], newThread);
  int key = newThread->tid % MAX_SIZE;
  l_insert(&allThreads[key], newThread);

  //L: store main context

  if (!mainRetrieved)
  {
    tcb *mainThread = (tcb*)malloc(sizeof(tcb));
    ucontext_t *mText = (ucontext_t*)malloc(sizeof(ucontext_t));
    getcontext(mText);
    mText->uc_link = &cleanup;

    mainThread->context = mText;
    mainThread->tid = 0;
    mainThread->priority = 0;
    mainThread->joinQueue = NULL;
    mainThread->jVal = NULL;
    mainThread->retVal = NULL;
    mainThread->status = READY;

    mainRetrieved = 1;

    l_insert(&allThreads[0], mainThread);

    currentThread = mainThread;

    
  }
  printf("New thread created: TID %d\n", newThread->tid);
  raise(SIGVTALRM);
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
  //L: call garbage collection
  currentThread->jVal = value_ptr;
  currentThread->status = EXIT;
  setcontext(&cleanup);
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr)
{
  //L: make sure thread can't wait on self
  if(thread == currentThread->tid)
  {return -1;}

  tcb *tgt = thread_search(thread);
  
  if(tgt == NULL)
  {
    return ESRCH; //L: sets errno to "no matching process ID found"
  }
  
  l_insert(&tgt->joinQueue, currentThread);

  currentThread->status = JOIN;

  raise(SIGVTALRM);

  *value_ptr = currentThread->retVal;

  return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
  //L: TODO: Undefined behavior if init called on locked mutex

  my_pthread_mutex_t m = *mutex;
  
  m.available = 1;
  m.locked = 0;
  m.holder = -1;
  m.queue = NULL;

  *mutex = m;
  return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex)
{
  my_pthread_mutex_t m = *mutex;

  if(!m.available)
  {return -1;}

  while(m.locked)
  {
    enqueue(&m.queue, currentThread);
    currentThread->status = MUTEX_WAIT;
    raise(SIGVTALRM);
  }

  m.locked = 1;
  m.holder = currentThread->tid;
  *mutex = m;

  return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex)
{
  //NOTE: handle errors: unlocking an open mutex, unlocking mutex not owned by calling thread, or accessing unitialized mutex

  my_pthread_mutex_t m = *mutex;

  if(!m.available || !m.locked || m.holder != currentThread->tid)
  {return -1;}

  m.locked = 0;
  m.holder = -1;

  tcb* muThread = dequeue(&m.queue);

  if(muThread != NULL)
  {enqueue(&runningQueue[muThread->priority], muThread);}

  *mutex = m;
  return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex)
{
  my_pthread_mutex_t m = *mutex;
  sigprocmask(SIG_BLOCK, &signal_set, NULL);
  //L: prevent threads from accessing while mutex is in destruction process
  m.available = 0;
  sigprocmask(SIG_UNBLOCK, &signal_set, NULL);  

  //L: if mutex still locked, wait for thread to release lock
  while(m.locked)
  {raise(SIGVTALRM);}

  //L: TODO: Is implementing a free-for-all post-destruction a terrible idea? This requires research...
  tcb *muThread;
  while(m.queue != NULL)
  {
    muThread = dequeue(&m.queue);
    enqueue(&runningQueue[muThread->priority], muThread);
    m.queue = m.queue->next;
  }

  *mutex = m;
  return 0;
};

void initializeQueues(list** runQ) 
{
  int i;
  for(i = 0; i < MAX_SIZE; i++) 
  {
    runningQueue[i] = NULL;
    allThreads[i] = NULL;
  }
  
}

