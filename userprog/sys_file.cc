#include <unistd.h>
#include "sys_file.h"
#include "kernel.h"
#include "main.h"
#include "synchconsole.h"

pair<OpenFile*, int> FileDescriptor[MAX_OPEN_FILES];

int findEmptySlot() {
    for (int i = 2; i < MAX_OPEN_FILES; i++)
        if (FileDescriptor[i].first == NULL)
            return i;
    return -1;
}


int SYS_Create(char* name) {

#ifdef FILESYS_STUB
    if (kernel->fileSystem->Create(name)) {
        DEBUG(dbgSys, "Create file successfully." << name);
        return 0;
    }
#else
    if (kernel->fileSystem->Create(name, 128)) {
        DEBUG(dbgSys, "Create file successfully." << name);
        return 0;
    }
#endif
    DEBUG(dbgSys, "Can't create file." << name);
    return -1;
}

void SYS_CloseAll() {
    for (int i = 2; i < MAX_OPEN_FILES; i++)
        if (FileDescriptor[i].first != NULL) {
            delete FileDescriptor[i].first;
            FileDescriptor[i].first = NULL;
            FileDescriptor[i].second = -1;
        }
}

int SYS_Seek(int position, OpenFileId id) {
    if (id <= 1 || id >= MAX_OPEN_FILES);
    OpenFile* file = FileDescriptor[id].first;
    if (FileDescriptor[id].second == SOCKET) {
        DEBUG(dbgSys, "Can't seek on socket.");
        return -1;
    }
    int length = file->Length();
    DEBUG(dbgSys, "Length file: " << length << " seek position: " << position);
    if (position > length || position < -1)
        return -1;
    if (position == -1)
        position = length;
    file->Seek(position);
    return position;
}



int SYS_Remove(char* name) {
    if (kernel->fileSystem->Remove(name)) {
        DEBUG(dbgSys, "Successfully remove " << name);
        return 0;
    }
    DEBUG(dbgSys, "Fail to remove " << name);
    return -1;
}

int SYS_Open(char* name, int type) {
    DEBUG(dbgSys, "Opening file " << name << "...");
    if (type != READ_WRITE || READ_ONLY) {
        DEBUG(dbgSys, "Wrong open type.");
        return -1;
    }
    OpenFile* file = kernel->fileSystem->Open(name);
    if (file) {
        // start from 2 as 0 and 1 are for console input and output
        int slot = findEmptySlot();
        if (slot != -1) {
            FileDescriptor[slot].first = file;
            FileDescriptor[slot].second = type;
            DEBUG(dbgSys, "Open successfully file " << name << " --- id: " << slot);
            return slot;
        }
        DEBUG(dbgSys, "System error: no available space to open file.");
        delete file;
    }
    DEBUG(dbgSys, "Fail to open file " << name);
    return -1;
}

int SYS_Read(char* buffer, int size, OpenFileId id) {
    if (id != Console_Input && (id <= 1 || id >= MAX_OPEN_FILES)) {
        DEBUG(dbgSys, "Unable to read: invalid open file index " << id << ".");
        return -1;
    }
    if (id == Console_Input) {
        char ch;
        int i;
        for (i = 0; i < size; i++) {
            ch = kernel->synchConsoleIn->GetChar();
            if (ch == EOF)
                break;
            buffer[i] = ch;
        }
        DEBUG(dbgSys, "Read from console " << i << " bytes.");
        return i;
    }
    else {
        OpenFile* file = FileDescriptor[id].first;
        int type = FileDescriptor[id].second;
        if (file) {
            int result;
            // can't use lseek on socket interface..
            if (type == SOCKET) {
#ifdef FILESYS_STUB
                result = read(file->getFileDescriptor(), buffer, size);
#else
                cerr << "Error: FILESYS_STUB is not defined" << endl;
                return -1;
#endif            
            }
            result = file->Read(buffer, size);
            DEBUG(dbgSys, "Read " << result << " bytes from file " << id);
            return result;
        }
        DEBUG(dbgSys, "Can't open file to read.");
    }
    return -1;
}

int SYS_Write(char* buffer, int size, OpenFileId id) {
    if (id != Console_Output && (id <= 1 || id >= MAX_OPEN_FILES)) {
        cerr << "Unable to write: invalid open file index " << id << "." << endl;
        return -1;
    }
    if (id == Console_Output) {
        DEBUG(dbgSys, "Write " << buffer << " to console.");
        for (int i = 0; i < size; i++) {
            kernel->synchConsoleOut->PutChar(buffer[i]);
        }
        return size;
    }
    else {
        OpenFile* file = FileDescriptor[id].first;
        int type = FileDescriptor[id].second;
        if (type == READ_ONLY) {
            DEBUG(dbgSys, "Unable to write, file is read-only.");
            return -1;
        }
        DEBUG(dbgSys, "Write " << buffer << " to file " << id);
        if (file) {
            int result = 0;
            if (type == SOCKET) {
                // can't use lseek on socket interface.
#ifdef FILESYS_STUB
                result = write(file->getFileDescriptor(), buffer, size);
#else
                cerr << "Error: FILESYS_STUB is not defined" << endl;
                return -1;
#endif
            }
            else
                result = file->Write(buffer, size);
            DEBUG(dbgSys, "Successfully write " << result << " bytes");
            return result;
        }
        else {
            DEBUG(dbgSys, "Unable to write, file is not open.");
        }
    }
    return -1;
}

int SYS_Close(OpenFileId id) {
    if (id <= 1 || id >= MAX_OPEN_FILES) {
        DEBUG(dbgSys, "ID exceeds file descriptor array, id: " << id << ".")
            return -1;
    }
    OpenFile* file = FileDescriptor[id].first;

    if (!file) {
        DEBUG(dbgSys, "File is not openning id: " << id);
        return -1;
    }
    DEBUG(dbgSys, "Successfully close file id: " << id);
    FileDescriptor[id].first = NULL;
    FileDescriptor[id].second = -1;
    delete file;
    return 0;
}