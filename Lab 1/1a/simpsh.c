// NAME: Shreya Raman
// EMAIL: shreyaraman99@gmail.com
// ID: 004923456

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

struct flags {
  int in, out, err, numArgs;
  char* cmd;
  char** args;
};

// checks that arg starts with two dashes
int hasDashes(char* arg) {
  char* dashes = "--";
  int len = strlen(arg);
  if (len >= 2) {
    char firstChars[3];
    strncpy(firstChars, arg, 2);
    firstChars[2] = 0;

    if (strcmp(firstChars, dashes) == 0) {
      return 1;
    }
  }
  return 0;
}

void doCmd(struct flags cmdArgs, int* FDarray) {
  int PID = fork();
  if (PID < 0) {
    fprintf(stderr, "Error: the fork failed.\n");
    exit(1);
  }
  // the child process
  else if (PID == 0) {
    int updStdin = FDarray[cmdArgs.in];
    close(0);
    dup2(updStdin, 0);
    close(updStdin);
    int updStdout = FDarray[cmdArgs.out];
    close(1);
    dup2(updStdout, 1);
    close(updStdout);
    int updStderr = FDarray[cmdArgs.err];
    close(2);
    dup2(updStderr, 2);
    close(updStderr);

    char* argmnts[cmdArgs.numArgs + 2];
    argmnts[0] = cmdArgs.cmd;

    for (int i = 1; i <= cmdArgs.numArgs; i++) {
      argmnts[i] = cmdArgs.args[i - 1];
    }

    argmnts[cmdArgs.numArgs + 1] = NULL;
    execvp(argmnts[0], argmnts);
  }
  else {
    //for later? parent process
  }
}

struct flags goCmdArgs(int argc, char** argv) {
  struct flags res;

  optind--;

  int c = 0;
  int cmdCount = 0;

  int firstOptind = -1;

  while (optind < argc) {
    char* curr = argv[optind];
    
    if (hasDashes(curr)) {
      break;
    }
    
    switch(c) {
    case 0:
      res.in = atoi(curr);
      break;
    case 1:
      res.out = atoi(curr);
      break;
    case 2:
      res.err = atoi(curr);
      break;
    case 3:
      res.cmd = curr;
      break;
    case 4:
      firstOptind = optind;
      cmdCount++;
      break;
    default:
      cmdCount++;
    }

    optind++;
    c++;
  }

  if (cmdCount > 0) {
    res.args = (char**) malloc(cmdCount* sizeof(char*));
    optind = firstOptind;
    int p = 0;
    
    while (optind < argc) {
      char* curr = argv[optind];
      
      if (hasDashes(curr)) {
	break;
      }
      
      res.args[p] = curr;
      optind++;
      p++;
    }
  }
  res.numArgs = cmdCount;

  return res;
}

int openF(char* fileName, int perm, int** FDs, int pos) {
  int openStat = 0;
  int fileDescriptor = open(fileName, perm);
  if (fileDescriptor < 0) {
    fprintf(stderr, "Error #%d: %s\n", errno, strerror(errno));
    fprintf(stderr, "Error from opening file: %s\n", fileName);
    openStat = 1;
  }

  (*FDs)[pos] = fileDescriptor;
  *FDs = realloc(*FDs, (pos + 2) * sizeof(int));

  return openStat;
}

int fileDesc(int in, int out, int err, int fileDescNum) {
  int isOkay = 1;
  if (err < 0 || err >= fileDescNum) {
    fprintf(stderr, "Error: invalid file descriptor for stderr: %d\n", err);
    isOkay = 0;
  }
  else if(in < 0 || in >= fileDescNum) {
    fprintf(stderr, "Error: invalid file descriptor for stdin: %d\n", in);
    isOkay = 0;
  }
  else if(out < 0 || out >= fileDescNum) {
    fprintf(stderr, "Error: invalid file descriptor for stdout: %d\n", out);
    isOkay = 0;
  }
  
  return isOkay;
}

int main(int argc, char** argv) {

  static struct option opts[] =
    {
     {"rdonly", required_argument, NULL, 'r'},
     {"wronly", required_argument, NULL, 'w'},
     {"command", required_argument, NULL, 'c'},
     {"verbose", no_argument, NULL, 'v'},
     {0, 0, NULL, 0}
    };
  
  int exitStat = 0;
  int verboseFlag = 0;
  int res;
  int* fileDescriptors = (int*) malloc(sizeof(int));
  int fileDescNum = 0;

  while ((res = getopt_long(argc, argv, "", opts, NULL)) != -1) {
      switch(res) {
      case 'r':
	if (verboseFlag) {
	  printf("--rdonly %s\n", optarg);
	}

	int rdonlyStat = openF(optarg, O_RDONLY, &fileDescriptors, fileDescNum);
	if (rdonlyStat) {
	  exitStat = 1;
	}
	fileDescNum++;
	break;
      case 'w':
	if (verboseFlag) {
	  printf("--wronly %s\n", optarg);
	}

	int wronlyStat = openF(optarg, O_WRONLY, &fileDescriptors, fileDescNum);
	if (wronlyStat) {
	  exitStat = 1;
	}
	fileDescNum++;
	break;
      case 'c':
	verboseFlag++;
	verboseFlag--;
	struct flags cmdArgs = goCmdArgs(argc, argv);
	if (verboseFlag) {
	  printf("--command %d %d %d %s", cmdArgs.in, cmdArgs.out, cmdArgs.err, cmdArgs.cmd);
	  for (int i = 0; i < cmdArgs.numArgs; i++) {
	    printf(" %s", cmdArgs.args[i]);
	  }
	  printf("\n");
	}

	int checkFDs = fileDesc(cmdArgs.in, cmdArgs.out, cmdArgs.err, fileDescNum);
	if (checkFDs) {
	  doCmd(cmdArgs, fileDescriptors);
	}
	else {
	  exitStat = 1;
	}

	if (cmdArgs.numArgs > 0) {
	  free(cmdArgs.args);
	}
	break;
      case 'v':
	verboseFlag = 1;
	break;
      default:
	fprintf(stderr, "Error: incorrect option");
	exitStat = 1;
	break;
      }
    }

    free(fileDescriptors);
    return exitStat;
}
