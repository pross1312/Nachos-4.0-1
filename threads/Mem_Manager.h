#ifndef MEM_MANAGER_H
#define MEM_MANAGER_H

#include "bitmap.h"
#include "synch.h"


class MemManager
{
public:
    MemManager();
    ~MemManager();

    int next_available();
    void free(int index);    
    size_t available();

private:
    Lock* lock;
    Bitmap* map; 
};


#endif // MEM_MANAGER_H
