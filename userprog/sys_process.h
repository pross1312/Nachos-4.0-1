#ifndef SYS_PROCESS_H
#define SYS_PROCESS_H

#include "main.h"


int SYS_Exec(const char* fileName);
int SYS_ExecV(int argc, char** argv);
void SYS_Exit(int exitCode);
int SYS_Join(int id);


#endif // SYS_PROCESS_H
