#ifndef COMMANDS_H
#define COMMANDS_H

extern int verboseFlag;
extern int FD;
extern int* FDs;
int nump;
typedef struct command {
  int filedescriptors[3];
  char** comm;
  int arguments;
  int pid;
} command;
extern command* pr;

int max(int a, int b);

void memErry();
void printerZ(char opt[]);
void printerO(const char opt[], const char args[]);
void printerT(char** command, int arguments);

int thruCommand(int argc, char* argv[], command* ans);
int goCommand(command* g);
int listCommand(command* g);
void freeCommand();
int waitHandler();

#endif
