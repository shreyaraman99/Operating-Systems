//NAME: Shreya Raman
//EMAIL: shreyaraman99@gmail.com
//ID: 004923456

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include "commands.h"
#include "files.h"


struct rusage us;
struct timeval startU;
struct timeval startS;

void startP() {
  int r = getrusage(RUSAGE_SELF, &us);
  if(!r) {
    startU = us.ru_utime;
    startS = us.ru_stime;
  }
  else
    fprintf(stderr, "Error %d\n", r);
}

void endP(char* n) {
  printf("--%s\t", n);
  int r = getrusage(RUSAGE_SELF, &us);
  if(!r) {
    long uDiffS = us.ru_utime.tv_sec - startU.tv_sec;
    int uDiffU = us.ru_utime.tv_usec - startU.tv_usec;
    long sDiffS = us.ru_stime.tv_sec - startS.tv_sec;
    int sDiffU = us.ru_stime.tv_usec - startS.tv_usec;

    printf("user: %lds %dus, ", uDiffS, uDiffU);
    printf("system: %lds, %dus\n", sDiffS, sDiffU);
    fflush(stdout);
  }
  else
    fprintf(stderr, "Error %d\n", r);
}

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
int profileFlag = 0;
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
      if (profileFlag)
	startP();
      fileFlags |= O_APPEND;
      if (profileFlag)
	endP("append");
    }
      break;
    case 'c': { //cloexec
      if(verboseFlag)
	printerZ("cloexec");
      if (profileFlag)
	startP();
      fileFlags |= O_CLOEXEC;
      if(profileFlag)
	endP("cloexec");
    }
      break;
    case 'g': { //creat
      if (verboseFlag)
	printerZ("creat");
      if (profileFlag)
	startP();
      fileFlags |= O_CREAT;
      if (profileFlag)
	endP("creat");
     
    }
      break;
    case 'd': { //directory                                                    
      if (verboseFlag)
        printerZ("directory");
      if (profileFlag)
	startP();
      fileFlags |= O_DIRECTORY;
      if(profileFlag)
	endP("directory");
    }
      break;
    case 'y': { //dsync                                                       
      if(verboseFlag)
        printerZ("dsync");
      if(profileFlag)
	startP();
      fileFlags |= O_DSYNC;
      if(profileFlag)
	endP("dsync");
    }
      break;
    case 'e': { //excl                                                         
      if (verboseFlag)
        printerZ("excl");
      if (profileFlag)
	startP();
      fileFlags |= O_EXCL;
      if(profileFlag)
	endP("excl");
      
    }
      break;
    case 'n': { //nofollow                                                     
      if (verboseFlag)
        printerZ("nofollow");
      if(profileFlag)
	startP();
      fileFlags |= O_NOFOLLOW;
      if(profileFlag)
	endP("nofollow");
    }
      break;
    case 'o': { //nonblock                                                     
      if(verboseFlag)
        printerZ("nonblock");
      if(profileFlag)
	startP();
      fileFlags |= O_NONBLOCK;
      if(profileFlag)
	endP("nonblock");
      
    }
      break;
    case 'r': { //rsync                                                        
      if (verboseFlag)
        printerZ("rsync");
      if(profileFlag)
	startP();
      fileFlags |= O_RSYNC;
      if(profileFlag)
	endP("rsync");
    }
      break;
    case 's': { //sync                                                        
      if (verboseFlag)
        printerZ("sync");
      if (profileFlag)
	startP();
      fileFlags |= O_SYNC;
      if (profileFlag)
	endP("sync");
    }
      break;
    case 't': { //trunc                                                        
      if (verboseFlag)
        printerZ("trunc");
      if (profileFlag)
	startP();
      fileFlags |= O_TRUNC;
      if (profileFlag)
	endP("trunc");
    }
      break;

    case 'z': { //rdonly
      if(verboseFlag)
	printerO("rdonly", optarg);
      if(profileFlag)
	startP();
      exitStat = max(exitStat, openFile(optarg, O_RDONLY));
      if(profileFlag)
	endP("rdonly");
    }
      break;
    case 'j': { //rdwr                                                        
      if(verboseFlag)
        printerO("rdwr", optarg);
      if(profileFlag)
	startP();
      exitStat = max(exitStat, openFile(optarg,O_RDWR));
      if(profileFlag)
	endP("rdwr");
    }
      break;
    case 'w': { //wronly                                                       
      if(verboseFlag)
        printerO("wronly", optarg);
      if(profileFlag)
	startP();
      exitStat = max(exitStat, openFile(optarg,O_WRONLY));
      if(profileFlag)
	endP("wronly");
    }
      break;
    case 'p': { //pipe                                                        
      if(verboseFlag)
        printerZ("pipe");
      if(profileFlag)
	startP();
      exitStat = max(exitStat, piping());
      if(profileFlag)
	endP("pipe");
    }
      break;

    case 'm': { //command                                                     
      command g;
      if(thruCommand(argc, argv, &g))
	exitStat = 1;
       if(profileFlag)
      	 startP();
       goCommand(&g);
       if(profileFlag)
      	 endP("command");
    }
      break;
    case 'q': { //wait                                                         
      if(verboseFlag)
	printerZ("wait");
      if(profileFlag)
	startP();
      exitStat = max(exitStat, waitHandler());
      if(profileFlag) {
	struct rusage us;
	getrusage(RUSAGE_CHILDREN, &us);

	printf("child process\t");
	printf("user: %lds %dus ", us.ru_utime.tv_sec, us.ru_utime.tv_usec);
	printf("system: %lds %dus\n", us.ru_stime.tv_sec, us.ru_stime.tv_usec);
	fflush(stdout);
	endP("wait");
      }
    }
      break;
    case 'k': { //close
      if(verboseFlag)
	printerO("close", optarg);
      if(profileFlag)
	startP();
      exitStat = max(exitStat, closeFile(optarg));
      if(profileFlag)
	endP("close");
    }
      break;
    case 'v': { //verbose
      if(verboseFlag)
	printerZ("verbose");
      if(profileFlag)
	startP();
      verboseFlag = 1;
      if(profileFlag)
	endP("verbose");
    }
      break;
    case 'f': { //profile
      if (verboseFlag)
	printerZ("profile");
      if(profileFlag)
	startP();
      if(profileFlag)
	endP("profile");
      profileFlag = 1;
    }
      break;
    case 'b': { //abort
      if(verboseFlag)
	printerZ("abort");
      if(profileFlag)
	startP();
      abortHandler();
      if(profileFlag)
	endP("abort");
    }
      break;
    case 'h': { //catch
      if(verboseFlag)
	printerO("catch", optarg);
      if(profileFlag)
	startP();
      int n = atoi(optarg);
      if(n < 0)
	break;
      signal(n, catchHandler);
      if(profileFlag)
	endP("catch");
    }
      break;
    case 'i': { //ignore
      if(verboseFlag)
	printerO("ignore", optarg);
      if(profileFlag)
	startP();
      int n = atoi(optarg);
      if(n < 0)
	break;
      signal(n, SIG_IGN);
      if(profileFlag)
	endP("ignore");
    }
      break;
    case 'u': { //default
      if(verboseFlag)
	printerO("default", optarg);
      if(profileFlag)
	startP();
      int n = atoi(optarg);
      if(n < 0)
	break;
      signal(n, SIG_DFL);
      if(profileFlag)
	endP("default");
    }
      break;
    case 'x': { //pause
      if(verboseFlag)
	printerZ("pause");
      if(profileFlag)
	startP();
      pause();
      if(profileFlag)
	endP("pause");
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
