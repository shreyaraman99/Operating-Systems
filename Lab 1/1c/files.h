#ifndef FILERS_H
#define FILERS_H

extern int fileFlags;
extern int FD;
extern int* FDs;

int openFile(char f[], int flag);
int closeFile(char* s);
int piping();
void printFD();

#endif
