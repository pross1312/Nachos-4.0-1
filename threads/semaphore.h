#include "synch.h"

#define MAX_SIZE_NAME 50
#define MAX_SEMAPHORE 10

class Sema {
private:
    Semaphore *_sem;
    char _name[MAX_SIZE_NAME];
public:
    Sema();
    ~Sema();
    void Create(char *name, int semval);    
    void Delete();                         
    void Wait();
    void Signal();
    char* GetName();
};
