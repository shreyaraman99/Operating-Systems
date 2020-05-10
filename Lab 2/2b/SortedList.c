#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
  if (opt_yield & INSERT_YIELD){ 
    sched_yield();
  }
  SortedListElement_t * ins = (SortedListElement_t *) malloc(sizeof(SortedListElement_t));
  if (ins == NULL){
    fprintf(stderr, "Error!\n");
    exit(EXIT_FAILURE);
  }
  ins->key = element->key;
  SortedListElement_t * cur = list;
  if ((cur->next)->key == NULL){
    list->next = ins;
    list->prev = ins;
    ins->next = list;
    ins->prev = list;
    return;
  }
  const char * s =(cur->next) -> key;
  const char * y = ins->key;
  while ((cur->next)->key != NULL && (strcmp(s, y) <= 0)){
    cur = cur->next;
  }

  SortedListElement_t * nel = cur->next;
  cur->next = ins;
  nel->prev = ins;
  ins->prev = cur;
  ins->next = nel;
}

int SortedList_delete( SortedListElement_t *element){
  if (opt_yield & DELETE_YIELD){ 
    sched_yield();
  }
  if (element->key == NULL){
    fprintf(stderr, "Error!\n");
    exit(2);
  }
  if ((element->next)->prev != element || (element->prev)->next != element){
    return 1;
  }

  (element->next)->prev = element->prev;
  (element->prev)->next = element->next;
  free(element); 
  return 0;
}


SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
  if (opt_yield & LOOKUP_YIELD){ 
    sched_yield();
  }
  SortedListElement_t * cur = list->next;

  while (cur->key != NULL){
    if (strcmp(cur->key, key) == 0){
      return cur;
    }
    cur = cur->next;
  }

  return NULL;
}

int SortedList_length(SortedList_t *list){
  if (opt_yield & LOOKUP_YIELD){ 
    sched_yield();
  }
  int len = 0;
  SortedListElement_t * cur = list->next; 
  while (cur->key != NULL) {
    len++;
    cur = cur->next;
  }
  return len;
}
