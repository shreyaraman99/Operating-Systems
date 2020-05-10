
// NAME: Shreya Raman
// EMAIL: shreyaraman99@gmail.com
// ID: 004923456

#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

void generate_segfault() {
  char* ptr = NULL;
  *ptr = 'A';
}

void signal_handler(int signalNum) {
  fprintf(stderr, "Segment fault occured, signal number: %d\n", signalNum);
  exit(4);
}

int main(int argc, char** argv) {
  int r = 0;
  int inf = 0; 
  int outf = 1;
  static struct option long_opts[] = 
    {
      {"input", required_argument, 0, 'i'},
      {"output", required_argument, 0, 'o'},
      {"segfault", no_argument, 0, 's'},
      {"catch", no_argument, 0, 'c'},
      {"dump-core", no_argument, 0, 'd'}
    };

  int segfaultFlag = 0;
  int catchFlag = 0;
  while ((r = getopt_long(argc, argv, "ioscd", long_opts, NULL)) != -1) {
    switch(r) {
    case 'i':
      if ((inf = open(optarg, O_RDONLY)) == -1) {
	perror("Error opening the input file");
	exit(2);
      }
      else {
	if (dup2(inf, STDIN_FILENO) == -1) {
	  perror("Error redirecting input file");
	  exit(2);
	}
      }
      break;
    case 'o':
      if ((outf = creat(optarg, 0666)) == -1) {
	perror("Error creating the output file");
	exit(3);
      }
      else {
	if (dup2(outf, STDOUT_FILENO) == -1) {
	  perror("Error redirecting output file");
	  exit(3);
	}
      }
      break;
    case 's':
      segfaultFlag = 1;
      break;
    case 'c':
      catchFlag = 1;
      break;
    case 'd':
      catchFlag = 0;
      break;
    default:
      fprintf(stderr, "Error with unrecognized character!");
      exit(1);
      break;
    }
  }

  if (segfaultFlag == 1)
    generate_segfault();

  if (catchFlag == 1)
    signal(SIGSEGV, signal_handler);

  char* buf;
  buf = (char*) malloc(sizeof(char));
  ssize_t stat = read(inf, buf, 1);
  while (stat > 0) {
    write(outf, buf, 1);
    stat = read(inf, buf, 1);
  }
  free(buf);

  exit(0);
}
