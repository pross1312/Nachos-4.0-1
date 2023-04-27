#include "main.h"
#include "Process.h"

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
    this->children     = new Table<Process>(MAX_RUNNING_PROCESS, this->name);
    this->lock         = new Lock("process synchronizer");
    this->space        = t->space;
    t->process         = this;

    // process console input and output
    // for now just set them to be kernel input and output
    procConsoleIn = kernel->synchConsoleIn;
    procConsoleOut = kernel->synchConsoleOut;


    DEBUG(dbgProc, "Create new process " << " id " << pid << " name " << main_thread->getName() << "\n");
    DEBUG(dbgProc, "Thread parent " << (p == NULL ? "None" : p->getName()) << "\n");

    // check if this is initProcess or not
    // if pid == 0 then it's initProcess, we won't put in in waiting queue but run it directly
    // else we fork to put the start process function on scheduler waiting queue and wait for its turn to run
    if (p == NULL) {
        main_thread->space->Execute(); 
    }
    else {
        ASSERT(p->addChild(this)):
        main_thread->Fork((VoidFunctionPtr)Process::start, this);
    }
}

Process::~Process()
{
    // don't delete consoleinput and output because process didn't own it, the same with thread
    // but for addrspace ...we will delete it
    delete this->lock;
    delete this->space;
    delete this->fTable;
    delete this->children;
    free(this->name);
}

bool Process::addChild(Process* child) {
    int index = this->children->add(child);
    return index != -1;
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
    bool result = false;
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
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
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
    DEBUG(dbgProc, "Starting process " << (int) p->getName() << " " << p->getName() << " id " << p->getId() << "\n");
    p->main_thread->space->InitRegisters();
    p->main_thread->space->RestoreState();
    p->setState(Running);
    kernel->machine->Run();
}
