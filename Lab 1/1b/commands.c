#include "commands.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>

int max(int a, int b) {
  return a > b ? a : b;
}

// PRINTERS                                                                     

void memErry() {
  fprintf(stderr, "Error: memory allocation\n");
  exit(1);
}

void printerZ(char opt[]) {
  printf("--%s\n", opt);
  fflush(stdout);
}

void printerO(const char opt[], const char args[]) {
  printf("--%s %s\n", opt, args);
  fflush(stdout);
}

void printerT(char** command, int arguments) {
  for (int i = 0; i < arguments; i++) {
    printf(" %s", command[i]);
  }
  printf("\n");
  fflush(stdout);
}



// COMMANDER                                                                    
int thruCommand(int argc, char* argv[], command* ans) {
  extern int optind;
  int ind = optind - 1;
  int anum = 0;
  int canum = -3;
  while (1) {
    char* c = argv[ind];
    if(ind >= argc || (c[0] == '-' && c[1] == '-')) {
      if(anum < 4) {
        fprintf(stderr, "--command: needs at least 4 arguments\n");
        return 1;
      }
      optind = ind;
      break;
    }

    if(anum < 3) {
      ans->filedescriptors[anum] = atoi(c);
      int filedescriptor = ans->filedescriptors[anum];
      if(filedescriptor < 0 || filedescriptor >= FD || FDs[filedescriptor] < 0) {
        fprintf(stderr, "Error: invalid FD\n");
        exit(1);
      }
      if (filedescriptor < 0) {
        fprintf(stderr, "--command: %d has to be a number", anum + 1);
        return 1;
      }
    }
    else if(anum == 3) {
      ans->comm = (char**)malloc(sizeof(char*));
      if(ans->comm == NULL)
        memErry();
      ans->comm[0] = c;
    }
    else {
      ans->comm = (char**)realloc(ans->comm, sizeof(char*) * (canum+1));
      if(ans->comm == NULL)
        memErry();
      ans->comm[canum] = c;
    }

    ind++;
    anum++;
    canum++;
  }
  ans->comm = (char**)realloc(ans->comm, sizeof(char*) * canum + 1);
  ans->comm[canum] = NULL;
  ans->arguments = canum;
  return 0;
}

int goCommand(command* g) {
  if (verboseFlag) {
    printf("--command");
    for (int i = 0; i < 3; i++) {
      printf(" %d", g->filedescriptors[i]);
    }
    printerT(g->comm, g->arguments);
  }
  int pid = fork();
  if(pid == 0) {
    for(int i = 0; i < 3; i++) {
      dup2(FDs[g->filedescriptors[i]], i);
    }
    for(int i = 0; i < FD; i++) {
      close(FDs[i]);
    }
    execvp(g->comm[0], g->comm);
    fprintf(stderr, "Error: invalid command\n");
    exit(1);
  }

  g->pid = pid;
  pr[nump] = *g;
  nump++;
  pr = (command*)realloc(pr, sizeof(command) * (nump+1));
  if(pr == NULL)
    memErry();
  return 0;
}

int listCommand(command* g) {
  for(int i = 0; i < g->arguments+1; i++) {
    fprintf(stderr, "cmd[%d] = %s\n", i, g->comm[i]);
  }
  return 0;
}

void freeCommand() {
  for(int i = 0; i < nump; i++) {
    free(pr[i].comm);
  }
  free(pr);
  nump = 0;
}

int waitHandler() {
  int stat;
  int exitStat = 0;
  for(int i = 0; i < nump; i++) {
    int pid = waitpid(pr[i].pid, &stat, 0);
    if(pid < 0) {
      fprintf(stderr, "Error: wait\n");
      exit(1);
    }
    if(WIFSIGNALED(stat)) {
      fprintf(stdout, "signal %d", WTERMSIG(stat));
      exitStat = max(WTERMSIG(stat) + 128, exitStat);
    }
    else if(WIFEXITED(stat)) {
      fprintf(stdout, "exit %d", WEXITSTATUS(stat));
      exitStat = max(WEXITSTATUS(stat), exitStat);
    }
    else {
      fprintf(stderr, "Error: in exiting\n");
    }
    printerT(pr[i].comm, pr[i].arguments);
  }
  freeCommand();
  pr = (command*)malloc(sizeof(command));
  if(FDs == NULL)
    memErry();
  return exitStat;
}
