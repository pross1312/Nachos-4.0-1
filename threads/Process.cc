#include "main.h"
#include "Process.h"
#include "debug.h"
#include "helper.h"


// this will duplicate the name so free it if you allocate that array
Process::Process(Process *p, Thread *t, const char* name)
{
    this->pid          = kernel->pTable->add(this); // add this process to processes table
    ASSERT(this->pid != -1);
    // this->state        = New;
    this->name         = strdup(name);
    this->parent       = p;
    this->main_thread  = t;
    this->fTable       = new Table<OpenFile>(MAX_OPEN_FILES, "open files management");
    this->children     = new List<Process*>;
    this->lock         = new Lock(const_cast<char*>("process synchronizer"));
    this->space        = t->space;
    this->joinsem      = new Semaphore(const_cast<char*>("join process semaphore"), 0);
    this->exitCode     = -1;
    this->joinid       = -1;
    this->isExited       = 0;

    this->argv         = NULL; // char**
    this->argc         = 0;
    this->vir_arg_addr = 0;

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
    DEBUG(dbgProc, "Process " << name << " is removed");
    // if isExited == true then these resources are deleted in Exit function already
    if (isExited == false) {
        delete this->space;
        this->main_thread->space = NULL;
        delete this->lock;
        delete this->joinsem;
        delete this->children; 
        if (argc != 0 && argv != NULL) {
            for (int i = 0; i < argc; i++)
                delete[] argv[i];
            delete[] argv;
        }
        this->procConsoleIn = NULL;
        this->procConsoleOut = NULL;
    }
    free(this->name);
}

void Process::addChild(Process* child) {
    this->lock->Acquire();
    ASSERT(!children->IsInList(child));
    children->Append(child);
    this->lock->Release();
}

void Process::removeChild(Process* child) {
    lock->Acquire();
    ASSERT(children->IsInList(child));
    children->Remove(child);
    lock->Release();
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
    if (p->getArgv() != NULL) {
        p->initArgument();
    }
    kernel->machine->Run();
}


Process* Process::createProcess(Process* p, Thread* t, const char* name) {
    if(kernel->pTable->checkFreeSlot()){
        return new Process(p, t, name);
    }
    DEBUG(dbgProc, "Full process");
    return NULL;
}

void Process::JoinWait(int joinid) {
    if (joinid == -1) return;
    this->joinid = joinid;
    joinsem->P();
}

void Process::JoinRelease(int joinid) {
    // no need lock because it can only join 1 process at 1 time
    // lock it and you will see terrible things happen...
    if (this->joinid != joinid) {
        DEBUG(dbgProc, "Parent " << this->name << " didn't join this process with id " << joinid);
        return;
    }
    DEBUG(dbgProc, "Parent " << this->name << " is released by process with id " << joinid);
    this->joinid = -1;
    this->joinsem->V();   
}

void Process::initArgument() {
    if (argv == NULL || argc == 0)
        return;
    int vir_stack = kernel->machine->ReadRegister(StackReg);
    ASSERT(vir_stack % 4 == 0);
    int size_argument_memory = argc * 4;

    vir_arg_addr = vir_stack - argc * 4; 

    for (int i = argc-1; i >= 0; i--) {
        int n_char = strlen(argv[i]);
        int n_aligned_size = n_char + (4 - n_char % 4);
        size_argument_memory += n_aligned_size;
        ASSERT(n_aligned_size % 4 == 0);
        ASSERT(vir_stack - size_argument_memory >= 0);
        int arg_addr = vir_stack - size_argument_memory;
        writeToMem((char*)&arg_addr, 4, vir_stack - ((argc - i) * 4)); 
        writeToMem(argv[i], n_char, arg_addr);
    }
    kernel->machine->WriteRegister(StackReg, vir_stack - size_argument_memory);
}

// remove all exited child
// if child has not exited, change its parent to the parent of this process
// also, we will delete some of this process resources too (addrspace, lock, semaphore, ...) since we no longer need them
// we will leave the name because its useful for debug
void Process::Exit(int exit)
{
    while (children->NumInList() > 0) {
        Process* child = children->RemoveFront();
        if (child->isExited == true) {
            DEBUG(dbgProc, "Child process " << child->getName() << " is already exited, remove it from pTable");
            ASSERT(kernel->pTable->remove(child->pid));
        }
        else {
            DEBUG(dbgProc, "Child process " << child->getName() << " hasn't exited yet, change parrent occur, new parent " << this->parent->getName());
            child->parent = this->parent;
            this->parent->addChild(child);
        }

    }
    isExited = true;
    exitCode = exit;
    
    DEBUG(dbgProc, "Free unnecessary resources for exited process");
    delete this->space;
    this->main_thread->space = NULL;
    delete this->lock;
    delete this->joinsem;
    delete this->children; 
    if (argc != 0 && argv != NULL) {
        for (int i = 0; i < argc; i++)
            delete[] argv[i];
        delete[] argv;
    }
    this->procConsoleIn = NULL;
    this->procConsoleOut = NULL;
}

