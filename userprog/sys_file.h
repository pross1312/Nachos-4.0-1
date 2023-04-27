#ifndef __USERPROG_SYS_FILE_H__ 
#define __USERPROG_SYS_FILE_H__ 
#include "syscall.h"

// const int MAX_OPEN_FILES = 20;



int SYS_Create(char* name);
int SYS_Remove(char* name);

OpenFileId SYS_Open(char* name, int type);
int SYS_Write(char* buffer, int size, OpenFileId id);
int SYS_Read(char* buffer, int size, OpenFileId id);
int SYS_Seek(int position, OpenFileId id);
int SYS_Close(OpenFileId id);





#endif
