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
int listFlag = 0;
int numt = 1;
SortedList_t * listHead;
SortedListElement_t * elmnts;
SortedListElement_t ** headArray = NULL;
int * spinArray = NULL;
int spinVar = 0;
pthread_mutex_t mLock;
pthread_mutex_t * mLockArray = NULL;
long long * threadsTimeElapsed;

unsigned long hash(const char *key) {
  unsigned long val = 5381;
  for(int i = 0; i < 3; i++)
    val = ((val << 5) + val) + key[i];
  return val;
}

void randomKeys(int c, SortedListElement_t * elmnts){
  srand((unsigned int)time(NULL));
  int len = 5;
  for (int i = 0 ; i < c; i ++){
    char * str = malloc((len + 1) * sizeof(char));
    for (int j = 0; j < len; j++){
      int offset = rand() % 26;
      str[j] = 'a' + offset;
    }
    str[len] = '\0';
    elmnts[i].key = str;
  }    
}

void * doThreads (void* arg){
  long sidx = (long) (arg);
  long i = numIterations * sidx;
  long init = i;
  for (; i< init + numIterations; i++){
    if (mutexFlag && !listFlag){ 
      struct timespec timeStart;
      clock_gettime(CLOCK_MONOTONIC, &timeStart);
      pthread_mutex_lock(&mLock);
      struct timespec timeEnd;
      clock_gettime(CLOCK_MONOTONIC, &timeEnd);
      long long timeElapsed = (timeEnd.tv_sec - timeStart.tv_sec) * 1000000000;
      timeElapsed += timeEnd.tv_nsec;
      timeElapsed -= timeStart.tv_nsec;
      threadsTimeElapsed[sidx] += timeElapsed;
      SortedList_insert(listHead, &elmnts[i]);
      pthread_mutex_unlock(&mLock);
    }
    else if (spinFlag && !listFlag){
      struct timespec timeStart;
      clock_gettime(CLOCK_MONOTONIC, & timeStart);
      while (__sync_lock_test_and_set(&spinVar, 1))
	;
      struct timespec timeEnd;
      clock_gettime(CLOCK_MONOTONIC, &timeEnd);
      long long timeElapsed = (timeEnd.tv_sec - timeStart.tv_sec) * 1000000000;
      timeElapsed += timeEnd.tv_nsec;
      timeElapsed -= timeStart.tv_nsec;
      threadsTimeElapsed[sidx] += timeElapsed;
      SortedList_insert(listHead, &elmnts[i]);
      __sync_lock_release(&spinVar);
    }
    else if (listFlag) {
      const char* theKey = elmnts[i].key;
      unsigned long hashVal = hash(theKey) % numt;
      if(spinFlag) {
	struct timespec timeStart;
	clock_gettime(CLOCK_MONOTONIC, &timeStart);
	while (__sync_lock_test_and_set(&spinArray[hashVal], 1))
	  ;
	struct timespec timeEnd;
	clock_gettime(CLOCK_MONOTONIC, &timeEnd);
	long long timeElapsed = (timeEnd.tv_sec - timeStart.tv_sec) * 1000000000;
	timeElapsed += timeEnd.tv_nsec;
	timeElapsed -= timeStart.tv_nsec;
	threadsTimeElapsed[sidx] += timeElapsed;
	SortedList_insert(headArray[hashVal], &elmnts[i]);
	__sync_lock_release(&spinArray[hashVal]);
      }
      else if (mutexFlag) {
	struct timespec timeStart;
        clock_gettime(CLOCK_MONOTONIC, &timeStart);
	pthread_mutex_lock(&mLockArray[hashVal]);
        struct timespec timeEnd;
        clock_gettime(CLOCK_MONOTONIC, &timeEnd);
        long long timeElapsed =(timeEnd.tv_sec- timeStart.tv_sec) * 1000000000;
        timeElapsed += timeEnd.tv_nsec;
        timeElapsed -= timeStart.tv_nsec;
        threadsTimeElapsed[sidx] += timeElapsed;
        SortedList_insert(headArray[hashVal], &elmnts[i]);
        pthread_mutex_unlock(&mLockArray[hashVal]);
      }
    }
    else{
      SortedList_insert(listHead, &elmnts[i]);
    }
  }
  long len;
  if (mutexFlag && !listFlag){
    struct timespec timeStart;
    clock_gettime(CLOCK_MONOTONIC, &timeStart);
    pthread_mutex_lock(&mLock);
    struct timespec timeEnd;
    clock_gettime(CLOCK_MONOTONIC, &timeEnd);
    long long timeElapsed = (timeEnd.tv_sec - timeStart.tv_sec) * 1000000000;
    timeElapsed += timeEnd.tv_nsec;
    timeElapsed -= timeStart.tv_nsec;
    threadsTimeElapsed[sidx] += timeElapsed;
    len = SortedList_length(listHead);
    pthread_mutex_unlock(&mLock);
  }
  else if (spinFlag && !listFlag){
    struct timespec timeStart;
    clock_gettime(CLOCK_MONOTONIC, & timeStart);
    while (__sync_lock_test_and_set(&spinVar, 1))
      ;
    struct timespec timeEnd;
    clock_gettime(CLOCK_MONOTONIC, &timeEnd);
    long long timeElapsed = (timeEnd.tv_sec - timeStart.tv_sec) * 1000000000;
    timeElapsed += timeEnd.tv_nsec;
    timeElapsed -= timeStart.tv_nsec;
    threadsTimeElapsed[sidx] += timeElapsed;
    len = SortedList_length(listHead);
    __sync_lock_release(&spinVar);
  }
  else if (listFlag) {
    len = 0;
    for(int j = 0; j < numt; j++) {
      if(spinFlag) {
	struct timespec timeStart;
	clock_gettime(CLOCK_MONOTONIC, &timeStart);
	while(__sync_lock_test_and_set(&spinArray[j], 1))
	  ;
	struct timespec timeEnd;
	clock_gettime(CLOCK_MONOTONIC, &timeEnd);
	long long timeElapsed = (timeEnd.tv_sec - timeStart.tv_sec) * 1000000000;
	timeElapsed += timeEnd.tv_nsec;
	timeElapsed -= timeStart.tv_nsec;
	threadsTimeElapsed[sidx] += timeElapsed;
	len += SortedList_length(headArray[j]);
	__sync_lock_release(&spinArray[j]);
      }
      else if (mutexFlag) {
	pthread_mutex_lock(&mLockArray[j]);
	len += SortedList_length(headArray[j]);
	pthread_mutex_unlock(&mLockArray[j]);
      }
    }
  }
  else{
    len = SortedList_length(listHead);
  }
    
  i = numIterations * sidx + len - len;
  init = i;
  for (; i < init + numIterations; i++){
    if (mutexFlag && !listFlag){
      struct timespec timeStart;
      clock_gettime(CLOCK_MONOTONIC, &timeStart);
      pthread_mutex_lock(&mLock);
      struct timespec timeEnd;
      clock_gettime(CLOCK_MONOTONIC, &timeEnd);
      long long timeElapsed = (timeEnd.tv_sec - timeStart.tv_sec) * 1000000000;
      timeElapsed += timeEnd.tv_nsec;
      timeElapsed -= timeStart.tv_nsec;
      threadsTimeElapsed[sidx] += timeElapsed;
      SortedListElement_t * ptr = SortedList_lookup(listHead, elmnts[i].key);
      if (ptr == NULL){
	fprintf(stderr, "Error: failed to lookup\n");
	exit(EXIT_FAILURE);
      }
      SortedList_delete(ptr);
      pthread_mutex_unlock(&mLock);
    }
    else if (spinFlag && !listFlag){
      struct timespec timeStart;
      clock_gettime(CLOCK_MONOTONIC, &timeStart);
      while (__sync_lock_test_and_set(&spinVar, 1))
	;
      struct timespec timeEnd;
      clock_gettime(CLOCK_MONOTONIC, &timeEnd);
      long long timeElapsed = (timeEnd.tv_sec - timeStart.tv_sec) * 1000000000;
      timeElapsed += timeEnd.tv_nsec;
      timeElapsed -= timeStart.tv_nsec;
      threadsTimeElapsed[sidx] += timeElapsed;
      SortedListElement_t * ptr = SortedList_lookup(listHead, elmnts[i].key);
      if (ptr == NULL){
	fprintf(stderr, "Error: failed to lookup\n");
	exit(EXIT_FAILURE);
      }
      SortedList_delete(ptr);
      __sync_lock_release(&spinVar);
    }
    else if (listFlag) {
      unsigned long iList = hash(elmnts[i].key) % numt;
      if(spinFlag) {
	struct timespec timeStart;
	clock_gettime(CLOCK_MONOTONIC, &timeStart);
	while (__sync_lock_test_and_set(&spinArray[iList], 1))
	  ;
	struct timespec timeEnd;
	clock_gettime(CLOCK_MONOTONIC, &timeEnd);
	long long timeElapsed =(timeEnd.tv_sec -timeStart.tv_sec) * 1000000000;
	timeElapsed += timeEnd.tv_nsec;
	timeElapsed -= timeStart.tv_nsec;
	threadsTimeElapsed[sidx] += timeElapsed;
	SortedListElement_t * ptr = SortedList_lookup(headArray[iList], elmnts[i].key);
	if (ptr == NULL){
	  fprintf(stderr, "Error: failed to lookup\n");
	  exit(EXIT_FAILURE);
	}
	SortedList_delete(ptr);
	__sync_lock_release(&spinArray[iList]);
      }
      else if (mutexFlag) {
	pthread_mutex_lock(&mLockArray[iList]);
	SortedListElement_t * ptr = SortedList_lookup(headArray[iList], elmnts[i].key);
	if(ptr == NULL) {
	  fprintf(stderr, "Error: failed to lookup\n");
	  exit(EXIT_FAILURE);
	}
	SortedList_delete(ptr);
	pthread_mutex_unlock(&mLockArray[iList]);
      }
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
    {"lists", required_argument, 0, 'l'},
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
    case 'l':
      listFlag = 1;
      numt = atoi(optarg);
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

  if (listFlag) {
    headArray = (SortedListElement_t **) malloc(numt * sizeof (SortedListElement_t *));
    for (int k = 0; k < numt; k++) {
      headArray[k] = (SortedList_t *) malloc(sizeof(SortedList_t));
      headArray[k]->key = NULL;
      headArray[k]->next = headArray[k];
      headArray[k]->prev = headArray[k];
    }
    if (spinFlag) {
      spinArray = (int*) malloc((numt * sizeof (int)));
      memset(spinArray, 0, numt * sizeof(int));
    }
    if(mutexFlag) {
      mLockArray = (pthread_mutex_t *) malloc(numt * sizeof (pthread_mutex_t));
      memset(mLockArray, 0, numt * sizeof(pthread_mutex_t));
    }
  }

  elmnts = (SortedListElement_t*) malloc( (numIterations * numThreads)* sizeof(SortedListElement_t));
    
  int num = numThreads * numIterations;
  randomKeys(num, elmnts);
 
  if (mutexFlag | spinFlag) {
    threadsTimeElapsed = (long long *) malloc(numThreads * sizeof(long));
    memset(threadsTimeElapsed, 0, numThreads * sizeof(long));
  }

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
   
  long long totalTime = 0;
  if (mutexFlag | spinFlag) {
    for (int b = 0; b < numThreads; b++) {
      totalTime += threadsTimeElapsed[b];
    }
  }

  long long numLock = (2 * numIterations + 1) * numThreads;
 
  if (SortedList_length(listHead) != 0){
    fprintf(stderr,"Error: size list != 0\n");
    exit(2);
  }

  char str [60];
  memset(str, 0, 60*sizeof(str[0]));
  char * yieldOpts = yield_opts();
  char * syncOpts = sync_opts();

  int numos = numThreads * numIterations * 3;
  long long avgTime = timeElapsed /numos;
  if (! (mutexFlag | spinFlag)) {
    sprintf(str, "list-%s-%s,%d,%d,%d,%d,%lld,%lld,0",yieldOpts, syncOpts, numThreads, numIterations, numt, numos, timeElapsed, avgTime);
  }
  else {
    long long clockTime = (totalTime / numLock);
    sprintf(str, "list-%s-%s,%d,%d,%d,%d,%lld,%lld,%lld", yieldOpts, syncOpts, numThreads, numIterations, numt, numos, timeElapsed, avgTime, clockTime);
  }
    
  printf("%s\n",str);
    
    
  if (mutexFlag){
    pthread_mutex_destroy(&mLock);
  }

  for (int i = 0; i < numThreads * numIterations; i++){
    free((void *)elmnts[i].key);
  }

  if (listFlag) {
    free(headArray);
    if(spinFlag)
      free(spinArray);
    if(mutexFlag)
      free(mLockArray);
  }
  free(elmnts);
  free(listHead);
  if (mutexFlag | spinFlag)
    free(threadsTimeElapsed);

  return 0;
    
}
