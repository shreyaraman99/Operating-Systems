#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <pthread.h>
#include <errno.h>


int opt_yield = 0;
int numThreads = 1;
int numIterations = 1;
int yieldFlag = 0;
int mutexFlag=0;
int spinFlag = 0;
int compFlag = 0;
int spinVar = 0;
pthread_mutex_t lock;

void add(long long *pointer, long long value)
{
  long long sum = *pointer + value;
  if(opt_yield) { 
    sched_yield(); 
  }
  *pointer = sum;
}

void pAdd(long long *pointer, long long value) //pointer is a pointer to counter
{

  if (mutexFlag){
    pthread_mutex_lock(&lock);
    add(pointer, value);
    pthread_mutex_unlock(&lock);
  }
  else if (spinFlag){
    while (__sync_lock_test_and_set(&spinVar, 1))
      while (spinVar)
	;
    add(pointer, value);
    __sync_lock_release(&spinVar);
  }

  else if (compFlag){
    long long aVal, eVal;
    do {
      eVal = * pointer;
      aVal = eVal + value;
      if (opt_yield){
	sched_yield();
      }
    }while(eVal != __sync_val_compare_and_swap(pointer, eVal, aVal));
  }
  else{
    add(pointer, value); //if no synchronization specified, do the peformAdd without protection
  }
}

void * wAdd(void* arg){
  long long * ptr = (long long *) arg;
  for (int i = 0; i < numIterations; i++){
    pAdd(ptr, 1);
  }
  for (int i = 0; i < numIterations; i++){
    pAdd(ptr, -1);
  }
  return NULL;
}


int main(int argc, char ** argv) {
    
  static struct option opts[] = {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
    {"yield", no_argument, 0, 'y'},
    {"sync", required_argument, 0, 's'},
    {0,0,0,0}
  };
  int ret;    
  long long count = 0;
  while ((ret = getopt_long(argc, argv, "", opts, 0)) != -1) {
    switch (ret) {
    case 't':
      numThreads = atoi(optarg);
      break;
    case 'i':
      numIterations = atoi(optarg);
      break;
    case 'y':
      opt_yield = 1;
      yieldFlag = 1;
      break;
    case 's':
      if (optarg[0] == 'm'){
	mutexFlag = 1;
      }
      else if (optarg[0] == 's')
	spinFlag = 1;
      else if (optarg[0] == 'c')
	compFlag = 1;

      break;
    default:
      fprintf(stderr, "Error: Unrecognized option\n");
      exit(EXIT_FAILURE);
      break;
    }
  }
  pthread_t threads[numThreads];
  if (mutexFlag){
    if (pthread_mutex_init(&lock, NULL) != 0)
      {
	fprintf(stderr, "Error: could not initialize mutex\n");
	exit(EXIT_FAILURE);
      }
  }
  struct timespec timeStart;
  clock_gettime(CLOCK_MONOTONIC, &timeStart);
  for (int i = 0; i < numThreads; i++){
    int c = pthread_create(&threads[i], NULL, wAdd, &count);
    if (c){
      fprintf(stderr, "Error: could not create the thread\n");
      exit(EXIT_FAILURE);
    }
  }
    
  for (int i = 0; i < numThreads; i ++){
    int j = pthread_join(threads[i], NULL);
    if (j != 0){ 
      fprintf(stderr, "Error: could not join thread. Error %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
    
  struct timespec timeEnd;
  clock_gettime(CLOCK_MONOTONIC, &timeEnd);
  long long timeElapsed =  (timeEnd.tv_sec - timeStart.tv_sec) * 1000000000; 
  timeElapsed += timeEnd.tv_nsec;
  timeElapsed -= timeStart.tv_nsec;

  char str [60];
  memset(str, 0, 60*sizeof(str[0]));

  int num = numThreads * numIterations * 2;
  long long avgTime = timeElapsed /num;
    

  if (!yieldFlag && mutexFlag){
    sprintf(str, "add-m,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations,num,timeElapsed, avgTime, count);
  }
  else if (!yieldFlag && spinFlag){
    sprintf(str, "add-s,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations, num, timeElapsed, avgTime, count);
  }
  else if (!yieldFlag && compFlag){
    sprintf(str, "add-c,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations, num, timeElapsed, avgTime, count);
  }
  else if (yieldFlag && mutexFlag) {
    sprintf(str, "add-yield-m,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations, num, timeElapsed, avgTime, count);
  }
  else if (yieldFlag && spinFlag){
    sprintf(str, "add-yield-s,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations, num, timeElapsed, avgTime, count);
  }
  else if (yieldFlag && compFlag){
    sprintf(str, "add-yield-c,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations, num, timeElapsed, avgTime, count);
  }
  else if (yieldFlag){
    sprintf(str, "add-yield-none,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations, num, timeElapsed, avgTime, count);
  }
  else{//nothing specicified
    sprintf(str, "add-none,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations,num, timeElapsed, avgTime, count);
  }
  printf("%s\n",str);
    
  if (mutexFlag){
    pthread_mutex_destroy(&lock);
  }
    
  return 0;
}
