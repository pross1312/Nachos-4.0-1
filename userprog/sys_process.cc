#include "sys_process.h"

// this will create process and pass arguments to it.
// DON'T free the memory allocated for arguments, it will be done when this process is deleted
int SYS_ExecV(int argc, char** argv) 
{
    if (argc < 1 || argv == NULL) {
        DEBUG(dbgProc, "Can't use ExecV process with no arguments");
        return -1;
    }
    AddrSpace* space = new AddrSpace;
    char* name = argv[0];
    if (space->Load(const_cast<char*>(name))) {
        Thread* t = new Thread("User created process");
        t->space = space;
        Process* p = Process::createProcess(kernel->currentThread->process, t, name);
        int id = -1;
        if(p != NULL) {
            id = p->getId();
            p->setArgv(argv);
            p->setArgc(argc);
            DEBUG(dbgProc, "Create process " << name << " with ID: " << id  << " and " << argc << " arguements");
            return id;
        }
        DEBUG(dbgProc, "Unable to create process " << name);
    }
    else {
        DEBUG(dbgProc, "Can't load AddrSpace\n");
    }
    return -1; 
}

int SYS_Exec(const char* name)
{
    AddrSpace* space = new AddrSpace;
    if (space->Load(const_cast<char*>(name))) {
        Thread* t = new Thread("User created process");
        t->space = space;
        Process* p= Process::createProcess(kernel->currentThread->process, t, name);
        int id = -1;
        if(p != NULL) {
            id = p->getId();
            DEBUG(dbgProc, "Create process " << name << " with ID: " << id);
            return id;
        }
        DEBUG(dbgProc, "Unable to create process " << name);
    }
    else {
        DEBUG(dbgProc, "Can't load AddrSpace\n");
    }
    return -1;
}

int SYS_Join(int id) 
{
    // turn off interrupt so that, nothing terrible can happen
    IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);

    Process* currentProcess = kernel->currentThread->process;
    if (id < 0 || id >= MAX_RUNNING_PROCESS) {
        DEBUG(dbgProc, "Invalid id " << id << ", ERROR: out of range");
        return -1;
    }

    if(!kernel->pTable->get(id)) {
        DEBUG(dbgProc, "No process with id " << id);
        return -1;
    }

    Process* childProcess = kernel->pTable->get(id);

    int processID = currentProcess->getId();
    if (childProcess->getParent()->getId() != processID) {
        DEBUG(dbgProc, "Invalid joiner, parent id and current process id don't match");
        return -1;
    }
    DEBUG(dbgProc, "Process " << currentProcess->getName() << " is waiting for " << kernel->pTable->get(id)->getName() << " to finish");
    currentProcess->JoinWait(id);
    int exitCode = childProcess->getExitCode(); // get joinee exit code (child exit code)
    DEBUG(dbgProc, "Process " << currentProcess->getName() << " continue");
    currentProcess->removeChild(childProcess);
    kernel->pTable->remove(id);
    
    (void)kernel->interrupt->SetLevel(oldLevel);
    ASSERT(kernel->interrupt->getLevel() == IntOn);
    return exitCode;
}

void SYS_Exit(int exitCode)
{

    // turn off interrupt so that this process's parent can't exit before it
    // it'll be turned on again after this process is remove entirely (by scheduler)
    // ... there's maybe a deadlock occur
    IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);  
    (void)oldLevel; // interrupt will be turned on by scheduler i think...

    Process* currentProcess = kernel->currentThread->process;

    // this is use to avoid possible memory error
    Thread* currentThread   = currentProcess->getThread();

    ASSERT(currentProcess != NULL);
    DEBUG(dbgProc, "Process " << currentProcess->getName() << " is exiting with code " << exitCode);
    Process* parent = currentProcess->getParent();

    
    if (parent == NULL) {
        DEBUG(dbgProc, "Init process exiting, stop machine");
        kernel->interrupt->Halt();
        ASSERTNOTREACHED();
    }

    DEBUG(dbgProc, "Process " << currentProcess->getName() << " is checking for children issues");

    // Exit function will move all the childs of this process to its parent process
    // then set some exit code, state and exit
    currentProcess->Exit(exitCode);
    


    DEBUG(dbgProc, "Process " << currentProcess->getName() << " has no child left, exiting");


    // awake its joined parent if necessary
    parent->JoinRelease(currentProcess->getId());

   

    // only finish this process thread but not delete its from the pTable
    currentThread->Finish();
    ASSERTNOTREACHED();
}
