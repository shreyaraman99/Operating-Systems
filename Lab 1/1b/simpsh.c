//NAME: Shreya Raman
//EMAIL: shreyaraman99@gmail.com
//ID: 004923456

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include "commands.h"
#include "files.h"

void catchHandler(int sig) {
  fprintf(stderr, "%d caught\n", sig);
  exit(sig);
}

void abortHandler() {
  int* a = 0;
  *a = 1;
}

int FD = 0;
int* FDs = 0;
int fileFlags = 0;
int verboseFlag = 0;
command* pr;

int main(int argc, char* argv[]) {
  static struct option opts[] = {
    {"append", no_argument, 0, 'a'},
    {"cloexec", no_argument, 0, 'c'},
    {"creat", no_argument, 0, 'g'},
    {"directory", no_argument, 0, 'd'},
    {"dysnc", no_argument, 0, 'y'},
    {"excl", no_argument, 0, 'e'},
    {"nofollow", no_argument, 0, 'n'},
    {"nonblock", no_argument, 0, 'o'},
    {"rsync", no_argument, 0, 'r'},
    {"sync", no_argument, 0, 's'},
    {"trunc", no_argument, 0, 't'},

    {"rdonly", required_argument, 0, 'z'},
    {"rdwr", required_argument, 0, 'j'},
    {"wronly", required_argument, 0, 'w'},
    {"pipe", no_argument, 0, 'p'},

    {"command", required_argument, 0, 'm'},
    {"wait", no_argument, 0, 'q'},

    {"close", required_argument, 0, 'k'},
    {"verbose", no_argument, 0, 'v'},
    {"profile", no_argument, 0, 'f'},
    {"abort", no_argument, 0, 'b'},
    {"catch", required_argument, 0, 'h'},
    {"ignore", required_argument, 0, 'i'},
    {"default", required_argument, 0, 'u'},
    {"pause", no_argument, 0, 'x'},
    {0, 0, 0, 0}
  };

  int r;
  int exitStat;
  int optIndex = 0;
  FDs = (int*)malloc(sizeof(int));
  if(FDs == NULL)
    memErry();
  pr = (command*)malloc(sizeof(command));
  if(FDs == NULL)
    memErry();

  while ((r = getopt_long(argc,argv, "", opts, &optIndex)) != -1) {
    switch(r) {
    case 'a': { //append
      if (verboseFlag)
	printerZ("append");
      fileFlags |= O_APPEND;
    }
      break;
    case 'c': { //cloexec
      if(verboseFlag)
	printerZ("cloexec");
      fileFlags |= O_CLOEXEC;
     
    }
      break;
    case 'g': { //creat
      if (verboseFlag)
	printerZ("creat");
      fileFlags |= O_CREAT;
     
    }
      break;
    case 'd': { //directory                                                    
      if (verboseFlag)
        printerZ("directory");
      fileFlags |= O_DIRECTORY;
      
    }
      break;
    case 'y': { //dsync                                                       
      if(verboseFlag)
        printerZ("dsync");
      fileFlags |= O_DSYNC;
      
    }
      break;
    case 'e': { //excl                                                         
      if (verboseFlag)
        printerZ("excl");
      fileFlags |= O_EXCL;
      
    }
      break;
    case 'n': { //nofollow                                                     
      if (verboseFlag)
        printerZ("nofollow");
      fileFlags |= O_NOFOLLOW;
      
    }
      break;
    case 'o': { //nonblock                                                     
      if(verboseFlag)
        printerZ("nonblock");
      fileFlags |= O_NONBLOCK;
      
    }
      break;
    case 'r': { //rsync                                                        
      if (verboseFlag)
        printerZ("rsync");
      fileFlags |= O_RSYNC;
      
    }
      break;
    case 's': { //sync                                                        
      if (verboseFlag)
        printerZ("sync");
      fileFlags |= O_SYNC;
      
    }
      break;
    case 't': { //trunc                                                        
      if (verboseFlag)
        printerZ("trunc");
      fileFlags |= O_TRUNC;
      
    }
      break;

    case 'z': { //rdonly
      if(verboseFlag)
	printerO("rdonly", optarg);
      exitStat = max(exitStat, openFile(optarg, O_RDONLY));
      
    }
      break;
    case 'j': { //rdwr                                                        
      if(verboseFlag)
        printerO("rdwr", optarg);
      exitStat = max(exitStat, openFile(optarg,O_RDWR));
      
    }
      break;
    case 'w': { //wronly                                                       
      if(verboseFlag)
        printerO("wronly", optarg);
      exitStat = max(exitStat, openFile(optarg,O_WRONLY));
      
    }
      break;
    case 'p': { //pipe                                                        
      if(verboseFlag)
        printerZ("pipe");
      exitStat = max(exitStat, piping());
      
    }
      break;

    case 'm': { //command                                                     
      command g;
      if(thruCommand(argc, argv, &g))
	exitStat = 1;
      else
	goCommand(&g);
     
    }
      break;
    case 'q': { //wait                                                         
      if(verboseFlag)
	printerZ("wait");
      exitStat = max(exitStat, waitHandler());
      
    }
      break;
    case 'k': { //close
      if(verboseFlag)
	printerO("close", optarg);
      exitStat = max(exitStat, closeFile(optarg));
    }
      break;
    case 'v': { //verbose
      if(verboseFlag)
	printerZ("verbose");
      verboseFlag = 1;
    }
      break;
    case 'b': { //abort
      if(verboseFlag)
	printerZ("abort");
      abortHandler();
    }
      break;
    case 'h': { //catch
      if(verboseFlag)
	printerO("catch", optarg);
      int n = atoi(optarg);
      if(n < 0)
	break;
      signal(n, catchHandler);
    }
      break;
    case 'i': { //ignore
      if(verboseFlag)
	printerO("ignore", optarg);
      int n = atoi(optarg);
      if(n < 0)
	break;
      signal(n, SIG_IGN);
    }
      break;
    case 'u': { //default
      if(verboseFlag)
	printerO("default", optarg);
      int n = atoi(optarg);
      if(n < 0)
	break;
      signal(n, SIG_DFL);
    }
      break;
    case 'x': { //pause
      if(verboseFlag)
	printerZ("pause");
      pause();
    }
      break;
    default:
      fprintf(stderr, "Error!");
    }
  }

  freeCommand();
  free(FDs);

  return exitStat;
}
