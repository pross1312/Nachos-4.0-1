#ifndef TABLE_H
#define TABLE_H

#include "openfile.h"
#include "synch.h"
#include "bitmap.h"


// class to store all some data
// this class is used to manage open files and processes

class Process;

template<class DATA>
class Table
{
public:
    Table() {}
    Table(int n, char const* debugname)
    {

        lock = new Lock(const_cast<char*>(debugname));
        nEntry = n;
        count = 0;
        table = new DATA*[n];
        for (int i = 0; i < nEntry; i++)
            table[i] = NULL;
    }

    // destructor will free all data in table
    ~Table()
    {
        for (int i = 0; i < nEntry; i++)
            if (table[i] != NULL)
                delete table[i];
        delete[] table;
        delete lock;
    }


    int add(DATA* p)
    {
        int result = -1;
        lock->Acquire();
        for (int i = 0; i < nEntry; i++)
            if (table[i] == NULL) {
                result = i;
                table[i] = p;
                count++;
                break;
            }
        lock->Release();
        return result;
    }
    bool checkFreeSlot(){
        if(count>nEntry){
            return 0;
        }
        return 1;
    }

    // remove will free the data at that position
    // return true we did remove something
    bool remove(int index)
    {
        ASSERT(index >= 0 && index < nEntry && "Index out of range");
        bool removed = false;
        lock->Acquire();
        if (table[index] != NULL) {
            delete table[index];
            table[index] = NULL;
            count--;
            removed = true;
        }
        lock->Release();
        return removed;
    }

    DATA* get(int index)
    {
        ASSERT(index >= 0 && index < nEntry && "Index out of range")
            DATA* result = NULL;
        lock->Acquire();
        result = table[index];
        lock->Release();
        return result;
    }

    int size() { return nEntry; }
    
    bool checkID(SpaceId id){
        if(table[id]==NULL){
            return 0;
        }
        return 1;
    }
private:
    Lock* lock;
    int nEntry;
    int count;
    DATA** table;
};


#endif // TABLE_H
