#include "files.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

int openFile(char f[], int flag) {
  int filedesc = open(f, flag | fileFlags, 0644);
  fileFlags = 0;
  FD++;
  FDs = (int*)realloc(FDs, sizeof(int) * FD);
  FDs[FD-1] = filedesc;
  if(filedesc<0) {
    fprintf(stderr, "Error: could not open %s\n", f);
    return 1;
  }
  return 0;
}

int closeFile(char* s) {
  int filedesc = atoi(s);
  if(filedesc < 0 || filedesc > FD - 1) {
    fprintf(stderr, "Error: invalid FD: %s\n", s);
    return 1;
  }

  if(close(FDs[filedesc]) < 0) {
    fprintf(stderr, "Error: could not close %d\n", filedesc);
    return 1;
  }
  FDs[filedesc] = -1;
  return 0;
}

int piping() {
  FD += 2;
  FDs = (int*)realloc(FDs, sizeof(int) * FD);
  if(pipe(FDs+FD - 2) < 0) {
    fprintf(stderr, "Error: could not open pipe\n");
    FDs[FD-2] = -1;
    FDs[FD-1] = -1;
    return 1;
  }
  return 0;
}

void printFD() {
  for (int i = 0; i < FD; i++) {
    fprintf(stderr, "FD[%d] = %d\n", i, FDs[i]);
  }
}
