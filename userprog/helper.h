#ifndef __USERPROG__HELPER
#define __USERPROG__HELPER

#include "kernel.h"
#include <stdio.h>


inline int readMemUntil(char* buffer, int vAddr, char end, int size) {
    int i = 0;
    for (; i < size; i++) {
        int ch;
        if (!kernel->machine->ReadMem(vAddr + i, 1, &ch))
            return -1;
        if (ch == end)
            break;
        buffer[i] = (char)ch;
    }
    return i;
}

inline bool readFromMem(char* buffer, int size, int virAddr) {
    int idx = 0;
    // memcpy(buffer, kernel->machine->mainMemory + virAddr, size);
    int data = 0;
    while (size >= 4) {
        if (!kernel->machine->ReadMem(virAddr + idx, 4, &data))
            return false;
        memcpy(buffer + idx, &data, 4);
        size -= 4;
        idx += 4;
    }
    while (size >= 2) {
        if (!kernel->machine->ReadMem(virAddr + idx, 2, &data))
            return false;
        memcpy(buffer + idx, &data, 2);
        size -= 2;
        idx += 2;
    }
    while (size >= 1) {
        if (!kernel->machine->ReadMem(virAddr + idx, 1, &data))
            return false;
        memcpy(buffer + idx, &data, 1);
        size -= 1;
        idx += 1;
    }
    return true;
}

inline void advancePC() {
    // advance program counter register
    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(NextPCReg));
    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(NextPCReg) + 4);
}


inline bool writeToMem(char* buffer, int size, int virAddr) {
    int count = 0;
    // memcpy(kernel->machine->mainMemory + virAddr, buffer, size);
    int data = 0;
    while (size >= 4) {
        memcpy(&data, buffer + count, 4);
        if (!kernel->machine->WriteMem(virAddr + count, 4, data))
            return false;
        size -= 4;
        count += 4;
    }
    while (size >= 2) {
        memcpy(&data, buffer + count, 2);
        if (!kernel->machine->WriteMem(virAddr + count, 2, data))
            return false;
        size -= 2;
        count += 2;
    }
    while (size >= 1) {
        memcpy(&data, buffer + count, 1);
        if (!kernel->machine->WriteMem(virAddr + count, 1, data))
            return false;
        size -= 1;
        count += 1;
    }

    return true;
}




#endif
