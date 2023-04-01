#include <unistd.h>
#include "sys_file.h"
#include "kernel.h"
#include "main.h"
#include "synchconsole.h"


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


int SYS_Seek(int position, OpenFileId id) {
    if (id <= 1 || id >= MAX_OPEN_FILES) {
        DEBUG(dbgSys, "Invalid id: " << id);
        return -1;
    }
    OpenFile* file = kernel->fileSystem->get(id);
    int length = file->Length();
    DEBUG(dbgSys, "Length file: " << length << " seek position: " << position);
    if (position > length || position < -1)
        return -1;
    if (position == -1)
        position = length;
    int result = file->Seek(position);
    if (result == -1) {
        DEBUG(dbgSys, "Seek error.");
        if (file->type() == SOCKET) {
            DEBUG(dbgSys, "Can't seek on socket.");
        }
    }
    return result;
}



int SYS_Remove(char* name) {
    if (kernel->fileSystem->Remove(name)) {
        DEBUG(dbgSys, "Successfully remove " << name);
        return 0;
    }
    return -1;
}

int SYS_Open(char* name, int type) {
    DEBUG(dbgSys, "Opening file " << name << "...");
    return kernel->fileSystem->Open(name, type);
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
        OpenFile* file = kernel->fileSystem->get(id);
        if (file) {
            int result = file->Read(buffer, size);
            if (result != -1) {
                DEBUG(dbgSys, "Read " << result << " bytes from file " << id);
            }
            else {
                DEBUG(dbgSys, "Can't read from file " << file->filePath() << ", type: " << file->type());
            }
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
        OpenFile* file = kernel->fileSystem->get(id);
        DEBUG(dbgSys, "Write " << buffer << " to file " << id);
        if (file) {
            int result = file->Write(buffer, size);
            if (result != -1) {
                DEBUG(dbgSys, "Successfully write " << result << " bytes");
            }
            else {
                DEBUG(dbgSys, "Can't write to file " << file->filePath() << ", type: " << file->type());
            }
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
    return kernel->fileSystem->Close(id) ? 0 : -1; 
}