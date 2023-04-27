#include "Mem_Manager.h"
extern const int NumPhysPages;

MemManager::MemManager()
{
    lock = new Lock(const_cast<char*>("MemManager"));
    map = new Bitmap(NumPhysPages);
    
}

MemManager::~MemManager()
{
    delete lock;
    delete map;
}

int MemManager::next_available()
{
    lock->Acquire();
    int result = map->FindAndSet();
    lock->Release();
    return result;
}

void MemManager::free(int index)
{
    lock->Acquire();
    map->Clear(index);
    lock->Release();
}

int MemManager::available()
{
    lock->Acquire();
    int result =  map->NumClear();
    lock->Release();
    return result;
}
