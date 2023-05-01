#include "synch.h"

#define MAX_SIZE 50
class Sema {
private:
    Semaphore *_sem;
    char _name[MAX_SIZE];
public:
    Sema();
    ~Sema();
    void Create(char *name, int semval);    
    void Delete();                         
    void Wait();
    void Signal();
    char* GetName();
};