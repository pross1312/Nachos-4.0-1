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

        mutex = new Semaphore(const_cast<char*>(debugname), 1);
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
        delete mutex;
    }


    int add(DATA* p)
    {
        int result = -1;
        mutex->P();
        for (int i = 0; i < nEntry; i++)
            if (table[i] == NULL) {
                result = i;
                table[i] = p;
                count++;
                break;
            }
        mutex->V();
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
        mutex->P();
        if (table[index] != NULL) {
            delete table[index];
            table[index] = NULL;
            count--;
            removed = true;
        }
        mutex->V();
        return removed;
    }

    DATA* get(int index)
    {
        ASSERT(index >= 0 && index < nEntry && "Index out of range")
            DATA* result = NULL;
        mutex->P();
        result = table[index];
        mutex->V();
        return result;
    }

    int size() { return nEntry; }
    
private:
    Semaphore* mutex;
    int nEntry;
    int count;
    DATA** table;
};


#endif // TABLE_H
