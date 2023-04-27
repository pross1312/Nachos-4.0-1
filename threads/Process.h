#ifndef PROCESS_H
#define PROCESS_H
#include "Table.h"
#include "list.h"
#include "openfile.h"
#include "synchconsole.h"


// This is class defined process control block for multilprogramming
//
#define MAX_OPEN_FILES 20
#define MAX_RUNNING_PROCESS 20

template<class DATA>
class Table;

enum ProcessState {
    New = 10, Running, Ready, Waiting, Terminated, Zombie
};

class Process
{
public:
    Process(Process* p, Thread* t, const char* name);
    ~Process();

    const char*  getName() { return name; }
    int          getId() { return pid; }
    ProcessState getState() { return state;  }
    void         setState(ProcessState s) { state = s; }

    bool       addChild(Process* child);
    OpenFile*  getFile(int id);
    int        addOpenFile(OpenFile* file);
    bool       isOpenFile(const char* name);
    bool       closeOpenFile(int index);

    SynchConsoleInput*  getConsoleInput() { return procConsoleIn; }
    SynchConsoleOutput* getConsoleOutput() { return procConsoleOut; }



private:
    static void start(Process*); // use to start process by fork


    // each process has its own console in, out, open files table, child processes and a unique id
    SynchConsoleInput* procConsoleIn;
    SynchConsoleOutput* procConsoleOut;
    Table<OpenFile>* fTable; 


    int pid;
    ProcessState state;
    char* name;
    AddrSpace* space;
    Thread* main_thread;
    Process* parent;
    Table<Process>* children;

    Lock* lock;
};


#endif // PROCESS_H
