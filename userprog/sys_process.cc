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
    Process* currentProcess = kernel->currentThread->process;
    if (id < 0 || id >= MAX_RUNNING_PROCESS) {
        DEBUG(dbgProc, "Invalid id " << id << ", ERROR: out of range");
        return -1;
    }
    if(!kernel->pTable->get(id)) {
        DEBUG(dbgProc, "No process with id " << id);
        return -1;
    }
    int processID = currentProcess->getId();
    if (kernel->pTable->get(id)->getParent()->getId() != processID) {
        DEBUG(dbgProc, "Invalid joiner, parent id and current process id don't match");
        return -1;
    }
    DEBUG(dbgProc, "Process " << currentProcess->getName() << " is waiting for " << kernel->pTable->get(id)->getName() << " to finish");
    currentProcess->JoinWait(id);
    int exitCode = currentProcess->getExitCode(); // get joinee exit code (child exit code)
    DEBUG(dbgProc, "Process " << currentProcess->getName() << " continue");
    return exitCode;
}

bool SYS_Exit(int exitCode)
{
    Process* currentProcess = kernel->currentThread->process;

    // this is use to avoid memory error
    // because after remove a process from table, that's process is already deleted so we can't access its thread afterward
    Thread* currentThread   = currentProcess->getThread();

    ASSERT(currentProcess != NULL);
    int processID = currentProcess->getId(); 
    DEBUG(dbgProc, "Process " << currentProcess->getName() << " is exiting with code " << exitCode);
    Process* parent = currentProcess->getParent();

    // currently, a process will wait until it has no childs left in order to exit
    DEBUG(dbgProc, "Process " << currentProcess->getName() << " is waiting until it has no child left");

    currentProcess->ExitWait();
    

    if (parent == NULL) {
        DEBUG(dbgProc, "Init process exiting, stop machine");
        kernel->interrupt->Halt();
        return true;

    } else {

        // turn off interrupt so that this process's parent can't exit before it
        IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);  

        DEBUG(dbgProc, "Process " << currentProcess->getName() << " has no child left, exiting");

        // first remove itself from its parent children list
        parent->removeChild(currentProcess);

        // awake its joined parent if necessary
        parent->JoinRelease(processID, exitCode);

        // kernel->pTable->get(processID)->DecNumWait();
        // this line will be called so that parent process can check if parent process can exit or not
        // see implementation in process.cc for more info
        DEBUG(dbgProc, "Call exit release for parent " << parent->getName());
        parent->ExitRelease();
        
        // remove process from pTable and finish thread
        DEBUG(dbgProc, "Remove process " << currentProcess->getName() << " ...");
        kernel->pTable->remove(processID);
        kernel->interrupt->SetLevel(oldLevel);
        currentThread->Finish();
        ASSERTNOTREACHED();
        return true;
    }
    return false;
}
