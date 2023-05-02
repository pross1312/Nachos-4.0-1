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

// enum ProcessState {
//     New = 10, Running, Ready, Waiting, Terminated, Zombie
// };
// enum ProcessStatus { PROCESS_JUST_CREATED, PROCESS_RUNNING, PROCESS_READY, 
// PROCESS_BLOCKED, PROCESS_ZOMBIE }; 

class Process
{
public:
    ~Process();

    // void         setState(ProcessState s) { state = s; }
    const char*  getName()     { return name; }
    int          getId()       { return pid; }
    // ProcessState getState()    { return state;  }
    Thread*      getThread()   { return main_thread;}
    Process*     getParent()   { return parent;}
    int          getExitCode() { return exitCode;}

    // void       DecNumWait();
    void       ExitRelease();
    void       JoinRelease(int joinid, int joinexitcode);
    void       ExitWait();
    void       JoinWait(int joinid);
    void       addChild(Process* child);
    void       removeChild(Process* child);
    OpenFile*  getFile(int id);
    int        addOpenFile(OpenFile* file);
    bool       isOpenFile(const char* name);
    bool       closeOpenFile(int index);
    char**     getArgv()                   { return argv; }
    void       setArgv(char** argv)        { this->argv = argv; }
    int        getArgc()                   { return argc; }
    void       setArgc(int argc)           { this->argc = argc; }
    int        getArgVirAddr()             { return this->vir_arg_addr; }

    SynchConsoleInput*  getConsoleInput()  { return procConsoleIn; }
    SynchConsoleOutput* getConsoleOutput() { return procConsoleOut; }

    static Process* createProcess(Process* p, Thread* t, const char* name);
    void initArgument();
private:
    Process(Process* p, Thread* t, const char* name);
    friend Process* createProcess(Process* p, Thread* t, const char* name);
    static void start(Process*); // use to start process by fork


    // each process has its own console in, out, open files table, child processes and a unique id
    SynchConsoleInput* procConsoleIn;
    SynchConsoleOutput* procConsoleOut;
    Table<OpenFile>* fTable; 

    int exitCode;
    int pid;
    // ProcessState state;
    char* name;
    AddrSpace* space;
    Thread* main_thread;
    Process* parent;
    List<Process*>* children;
    Semaphore* joinsem;
    SpaceId joinid;
    int vir_arg_addr; // this is address that point to the argv array
    // int numwait;
    int isExit;
    
    char** argv;
    int argc;

    Semaphore* exitsem;
    Lock* lock;
};


#endif // PROCESS_H
