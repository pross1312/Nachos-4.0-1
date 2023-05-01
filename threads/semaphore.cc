#include "semaphore.h"

Sema::Sema() {
    memset(_name, 0, MAX_SIZE);
    _sem = NULL;
}

Sema::~Sema() {
    if (_sem != NULL) delete _sem;
}

void Sema::Create(char *name, int semval) {
    if (name == NULL || semval < 0 || _sem != NULL) return;
    strncpy(_name, name, MAX_SIZE-1);
    _sem = new Semaphore(_name, semval);
}

void Sema::Delete() {
    if (_sem == NULL) return;
    memset(_name, 0, MAX_SIZE);
    delete _sem;
    _sem = NULL;
}

void Sema::Wait() {
    if (_sem != NULL) _sem->P();
}

void Sema::Signal() {
    if (_sem != NULL) _sem->V();
}

char* Sema::GetName() {
    return _name;
}