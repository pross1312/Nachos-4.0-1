#include "main.h"
#include "Process.h"
#include "debug.h"

// this will duplicate the name so free it if you allocate that array
Process::Process(Process *p, Thread *t, const char* name)
{
    this->pid          = kernel->pTable->add(this); // add this process to processes table
    ASSERT(this->pid != -1);
    this->state        = New;
    this->name         = strdup(name);
    this->parent       = p;
    this->main_thread  = t;
    this->fTable       = new Table<OpenFile>(MAX_OPEN_FILES, "open files management");
    this->children     = new List<Process*>;
    this->lock         = new Semaphore("process synchronizer", 1);
    this->space        = t->space;
    this->joinsem      = new Semaphore("join process semaphore", 0);
    this->exitsem      = new Semaphore("exit process semaphore", 0);
    this->exitCode     = -1;
    this->joinid       = -1;
    t->process         = this;


    // process console input and output
    // for now just set them to be kernel input and output
    procConsoleIn = kernel->synchConsoleIn;
    procConsoleOut = kernel->synchConsoleOut;


    DEBUG(dbgProc, "Create new process " << this->name << " id " << this->pid << " parent " << (parent == NULL ? "NULL" : parent->getName()));

    // check if this is initProcess or not
    // if pid == 0 then it's initProcess, we won't put in in waiting queue but run it directly
    // else we fork to put the start process function on scheduler waiting queue and wait for its turn to run
    if (p == NULL) {
        main_thread->space->Execute(); 
    }
    else {
        p->addChild(this);
        main_thread->Fork((VoidFunctionPtr)Process::start, this);
    }
}

Process::~Process()
{
    // don't delete consoleinput and output because process didn't own it, the same with thread
    // but for addrspace ...we will delete it
    delete this->lock;
    delete this->space;
    this->main_thread->space = NULL;
    delete this->fTable;
    delete this->children;
    delete this->joinsem;
    delete this->exitsem;
    free(this->name);
}

void Process::addChild(Process* child) {
    this->lock->P();
    ASSERT(!children->IsInList(child));
    children->Append(child);
    this->lock->V();
}

void Process::removeChild(Process* child) {
    lock->P();
    ASSERT(children->IsInList(child));
    children->Remove(child);
    lock->V();
}

int Process::addOpenFile(OpenFile* file) {
    int result = fTable->add(file);
    // like you see 
    if (result == -1)
        return -1;
    // 0 and 1 are virtually stored console input and output
    // so we add 2 here and -2 in getFile
    return result + 2;
}

bool Process::closeOpenFile(int index) {
    ASSERT(index >= 2 && index < MAX_OPEN_FILES);
    return fTable->remove(index);
}

bool Process::isOpenFile(const char* name) {
    char* full_path = realpath(name, NULL);
    if (full_path == NULL) {
        DEBUG(dbgFile, "Check open file error: can't find full path.");
        return false;
    }
    
    bool result = false;
    // process file table is now handling console in and out so file start from
    for (int i = 2; i < MAX_OPEN_FILES; i++) {
        OpenFile* file = kernel->currentThread->process->getFile(i);
        if (file && strcmp(file->filePath(), full_path) == 0) {
            result = true;
            break;
        }
    }
    free(full_path);
    return result;
}

OpenFile* Process::getFile(int id) {
    DEBUG(dbgFile, "Trying to get file " << id);
    ASSERT(id >= 2 && id < MAX_OPEN_FILES);
    return fTable->get(id - 2);
}


void Process::start(Process* p)
{
    DEBUG(dbgProc, "Starting process " << p->getName() << " id " << p->getId() << "\n");
    p->main_thread->space->InitRegisters();
    p->main_thread->space->RestoreState();
    p->setState(Running);
    kernel->machine->Run();
}

Process* Process::createProcess(Process* p, Thread* t, const char* name){
    if(kernel->pTable->checkFreeSlot()){
        return new Process(p, t, name);
    }
    DEBUG(dbgProc, "Full process");
    return NULL;
}

void Process::JoinWait(int joinid){
    if (joinid == -1) return;
    this->joinid = joinid;
    joinsem->P();
}

void Process::ExitWait(){
    if(children->NumInList() > 0){
        isExit = 1;
        exitsem->P();
    }
}
// 1 -> 2 -> 3
// 2: exit
// 2 doi con cua exit het 


void Process::ExitRelease() {
    if (children->NumInList() == 0 && isExit == 1)
        exitsem->V();
}


void Process::JoinRelease(int joinid, int exitCode) {
    // this means that parent isn't waiting for this child or something went horribly wrong
    if (this->joinid != joinid) return; 
    this->joinid = -1;
    this->exitCode = exitCode;
    this->joinsem->V();
}

// void Process::DecNumWait() {
//     if (this->numwait > 0)
//         --this->numwait;
// }
