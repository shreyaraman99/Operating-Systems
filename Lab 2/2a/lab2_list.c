#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include "SortedList.h"

int numThreads = 1;
int numIterations = 1;
int spinFlag = 0;
int mutexFlag = 0;
int opt_yield = 0;
int yieldFlag = 0;
SortedList_t * listHead;
SortedListElement_t * elmnts;
int spinVar = 0;
pthread_mutex_t mLock;


void randomKeys(int c, SortedListElement_t * elmnts){
  srand((unsigned int)time(NULL));
  int len = 5;
  for (int i = 0 ; i < c; i ++){
    char * str = malloc((len + 1) * sizeof(char));
    for (int j = 0; j < len; j++){
      int offset = rand() %26;
      str[j] = 'a' + offset;
    }
    str[len] = '\0';
    elmnts[i].key = str;
  }    
}

void * doThreads (void* arg){
  long sidx = (long) (arg);
  long i = numThreads * sidx;
  for (; i< numIterations; i++){
    if (mutexFlag){ 
      pthread_mutex_lock(&mLock);
      SortedList_insert(listHead, &elmnts[i]);
      pthread_mutex_unlock(&mLock);
    }
    else if (spinFlag){
      while (__sync_lock_test_and_set(&spinVar, 1))
	;
      SortedList_insert(listHead, &elmnts[i]);
      __sync_lock_release(&spinVar);
    }
    else{
      SortedList_insert(listHead, &elmnts[i]);
    }
  }
  long len;
  if (mutexFlag){
    pthread_mutex_lock(&mLock);
    len = SortedList_length(listHead);
    pthread_mutex_unlock(&mLock);
  }
  else if (spinFlag){
    while (__sync_lock_test_and_set(&spinVar, 1))
      ;
    len = SortedList_length(listHead);
    __sync_lock_release(&spinVar);
  }
  else{
    len = SortedList_length(listHead);
  }
    
  i = numThreads * sidx + len - len;
  for (; i < numIterations; i++){
    if (mutexFlag){
      pthread_mutex_lock(&mLock);
      SortedListElement_t * ptr = SortedList_lookup(listHead, elmnts[i].key);
      if (ptr == NULL){
	fprintf(stderr, "Error: failed to lookup\n");
	exit(EXIT_FAILURE);
      }
      SortedList_delete(ptr);
      pthread_mutex_unlock(&mLock);
    }
    else if (spinFlag){
      while (__sync_lock_test_and_set(&spinVar, 1))
	;
      SortedListElement_t * ptr = SortedList_lookup(listHead, elmnts[i].key);
      if (ptr == NULL){
	fprintf(stderr, "Error: failed to lookup\n");
	exit(EXIT_FAILURE);
      }
      SortedList_delete(ptr);
      __sync_lock_release(&spinVar);
    }
        
    else{
      SortedListElement_t * ptr = SortedList_lookup(listHead, elmnts[i].key);
      if (ptr == NULL){
	fprintf(stderr, "Error: failed to lookup\n");
	exit(EXIT_FAILURE);
      }
      SortedList_delete(ptr);
    }     
  }
  return NULL;
}

char * yield_opts (){
  if ((opt_yield & INSERT_YIELD) == 0 && (opt_yield & DELETE_YIELD) == 0 && (opt_yield & LOOKUP_YIELD) == 0){
    return "none";
  }
  else if ((opt_yield & INSERT_YIELD) != 0 && (opt_yield & DELETE_YIELD) == 0 && (opt_yield & LOOKUP_YIELD) == 0){
    return "i";
  }
  else if ((opt_yield & INSERT_YIELD) == 0 && (opt_yield & DELETE_YIELD) == 0 && (opt_yield & LOOKUP_YIELD) != 0){
    return "l";
  }
  else if((opt_yield & INSERT_YIELD) == 0 && (opt_yield & DELETE_YIELD) != 0 && (opt_yield & LOOKUP_YIELD) == 0){
    return "d";
  }
  else if ((opt_yield & INSERT_YIELD) != 0 && (opt_yield & DELETE_YIELD) != 0 && (opt_yield & LOOKUP_YIELD) == 0){
    return "id";
  }
  else if((opt_yield & INSERT_YIELD) != 0 && (opt_yield & DELETE_YIELD) == 0 && (opt_yield & LOOKUP_YIELD) != 0){
    return "il";
  }
  else if ((opt_yield & INSERT_YIELD) == 0 && (opt_yield & DELETE_YIELD) != 0 && (opt_yield & LOOKUP_YIELD) != 0){
    return "dl";
  }
  else if ((opt_yield & INSERT_YIELD) != 0 && (opt_yield & DELETE_YIELD) != 0 && (opt_yield & LOOKUP_YIELD) != 0){
    return "idl";
  }
  return NULL;
    
}

char * sync_opts(){
  if (spinFlag){
    return "s";
  }
  else if (mutexFlag){
    return"m";
  }
  else {
    return "none";
  }
  return NULL;
}


void segfaultHandler(int sigNum){
  fprintf(stderr, "Segfault: opt_yield=%d, numThreads = %d, numIterations = %d, signum = %d\n", opt_yield, numThreads, numIterations,sigNum);
  exit(2);
}


int main(int argc, char ** argv) {
    
  static struct option opts[] = {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
    {"yield", required_argument, 0, 'y'},
    {"sync", required_argument, 0, 's'},
    {0,0,0,0}
  };
  int ret;
  signal(SIGSEGV, segfaultHandler);
    
  while ((ret = getopt_long(argc, argv, "", opts, 0)) != -1) {
    switch (ret) {
    case 't':
      numThreads = atoi(optarg);
      break;
    case 'i':
      numIterations = atoi(optarg);
      break;
    case 'y':
      for (unsigned int i = 0; i < strlen(optarg); i++){
	if (optarg [i] == 'i'){
	  opt_yield = opt_yield | INSERT_YIELD;
	}
	else if (optarg[i] == 'd'){
	  opt_yield = opt_yield | DELETE_YIELD;
	}
	else if (optarg [i] == 'l') {
	  opt_yield = opt_yield | LOOKUP_YIELD;
	}
	else{
	  fprintf(stderr, "Unrecognized option\n");
	  exit(EXIT_FAILURE);
	}
      }
      break;
    case 's':
      if (optarg[0] == 'm'){
        mutexFlag = 1;
      }
      else if (optarg[0] == 's')
        spinFlag = 1;
      else{
        fprintf(stderr, "Unexpected value\n");
        exit(EXIT_FAILURE);
      }
      break;
    default:
      fprintf(stderr, "Unrecognized option\n");
      exit(EXIT_FAILURE);
      break;
    }
  }
  
    if (mutexFlag){
    pthread_mutex_init(&mLock, NULL);
  }
    
  listHead = (SortedList_t *)malloc(sizeof(SortedList_t));
  listHead->key = NULL;
  listHead -> next = listHead;
  listHead -> prev = listHead;

  elmnts = (SortedListElement_t*) malloc( (numIterations * numThreads)* sizeof(SortedListElement_t));
    
  int num = numThreads * numIterations;
  randomKeys(num, elmnts);
 
  struct timespec timeStart;
  clock_gettime(CLOCK_MONOTONIC, &timeStart);
 
  pthread_t threads [numThreads];
  for (long i = 0; i < numThreads; i ++){
    int c = pthread_create(&threads[i], NULL, doThreads, (void *)i);
    if (c){
      fprintf(stderr, "Error: could not create the thread\n");
      exit(2);
    }
  }
  for (int i = 0; i < numThreads; i ++){

    int j = pthread_join(threads[i], NULL);
    if (j != 0){ 
      fprintf(stderr, "Error: could not join thread. Error %s\n", strerror(errno));
      exit(2);
    }
  }
    
  struct timespec timeEnd;
  clock_gettime(CLOCK_MONOTONIC, &timeEnd);
  long long timeElapsed =  (timeEnd.tv_sec - timeStart.tv_sec) * 1000000000; 
  timeElapsed += timeEnd.tv_nsec;
  timeElapsed -= timeStart.tv_nsec;
    
  if (SortedList_length(listHead) != 0){
    fprintf(stderr,"Error: size list != 0\n");
    exit(2);
  }

  char str [60];
  memset(str, 0, 60*sizeof(str[0]));
  char * yieldOpts = yield_opts();
  char * syncopts = sync_opts();

  int numos = numThreads * numIterations * 3;
  long long avgTime = timeElapsed /numos;
  sprintf(str, "list-%s-%s,%d,%d,1,%d,%lld,%lld",yieldOpts, syncopts, numThreads, numIterations,numos, timeElapsed, avgTime);
    
  printf("%s\n",str);
    
    
  if (mutexFlag){
    pthread_mutex_destroy(&mLock);
  }

  for (int i = 0; i < numThreads * numIterations; i++){
    free((void *)elmnts[i].key);
  }
  free(elmnts);
  free(listHead);

  return 0;
    
}
