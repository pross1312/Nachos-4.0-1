#include <unistd.h>
#include "sys_file.h"
#include "kernel.h"
#include "main.h"
#include "synchconsole.h"


int SYS_Create(char* name) {

#ifdef FILESYS_STUB
    if (kernel->fileSystem->Create(name)) {
        DEBUG(dbgFile, "Create file successfully." << name);
        return 0;
    }
#else
    if (kernel->fileSystem->Create(name, 128)) {
        DEBUG(dbgFile, "Create file successfully." << name);
        return 0;
    }
#endif
    DEBUG(dbgFile, "Can't create file." << name);
    return -1;
}


int SYS_Seek(int position, OpenFileId id) {
    if (id <= 1 || id >= MAX_OPEN_FILES) {
        DEBUG(dbgFile, "Invalid id: " << id);
        return -1;
    }
    OpenFile* file = kernel->currentThread->process->getFile(id);
    int length = file->Length();
    DEBUG(dbgFile, "Length file: " << length << " seek position: " << position);
    if (position > length || position < -1)
        return -1;
    if (position == -1)
        position = length;
    int result = file->Seek(position);
    if (result == -1) {
        DEBUG(dbgFile, "Seek error.");
        if (file->type() == SOCKET) {
            DEBUG(dbgFile, "Can't seek on socket.");
        }
    }
    return result;
}


int SYS_Remove(char* name) {
    if (kernel->currentThread->process->isOpenFile(name)) {
        DEBUG(dbgFile, "File " <<name << " is open");
        return -1; 
    }
    DEBUG(dbgFile, "File " << name << " isn't open, removing");
    if (kernel->fileSystem->Remove(name)) {
        DEBUG(dbgFile, "Successfully remove " << name);
        return 0;
    }
    return -1;
}

int SYS_Open(char* name, int type) {
    DEBUG(dbgFile, "Opening file " << name << "...");
    OpenFile* file = kernel->fileSystem->Open(name, type);
    if (file == NULL) {
        return -1;
    }
    int index = kernel->currentThread->process->addOpenFile(file);
    if (index == -1) {
        DEBUG(dbgFile, "Can't open file " << name << ", table is full");
        delete file;
        return -1;
    }
    DEBUG(dbgFile, "Successfully open file " << name << "\n");
    return  index;
}

int SYS_Read(char* buffer, int size, OpenFileId id) {
    if (id != Console_Input && (id <= 1 || id >= MAX_OPEN_FILES)) {
        DEBUG(dbgFile, "Unable to read: invalid open file index " << id << ".");
        return -1;
    }
    if (id == Console_Input) {
        char ch;
        int i;
        for (i = 0; i < size; i++) {
            ch = kernel->currentThread->process->getConsoleInput()->GetChar();
            if (ch == EOF)
                break;
            buffer[i] = ch;
        }
        DEBUG(dbgFile, "Read from console " << i << " bytes.");
        return i;
    }
    else {
        OpenFile* file = kernel->currentThread->process->getFile(id);
        if (file) {
            int result = file->Read(buffer, size);
            if (result != -1) {
                DEBUG(dbgFile, "Read " << result << " bytes from file " << id);
            }
            else {
                DEBUG(dbgFile, "Can't read from file " << file->filePath() << ", type: " << file->type());
            }
            return result;
        }
        DEBUG(dbgFile, "Can't open file to read.");
    }
    return -1;
}

int SYS_Write(char* buffer, int size, OpenFileId id) {
    if (id != Console_Output && (id <= 1 || id >= MAX_OPEN_FILES)) {
        cerr << "Unable to write: invalid open file index " << id << "." << endl;
        return -1;
    }
    if (id == Console_Output) {
        DEBUG(dbgFile, "Write " << buffer << " to console.");
        for (int i = 0; i < size; i++) {
            kernel->currentThread->process->getConsoleOutput()->PutChar(buffer[i]);
        }
        return size;
    }
    else {
        OpenFile* file = kernel->currentThread->process->getFile(id);
        DEBUG(dbgFile, "Write " << buffer << " to file " << id);
        if (file) {
            int result = file->Write(buffer, size);
            if (result != -1) {
                DEBUG(dbgFile, "Successfully write " << result << " bytes");
            }
            else {
                DEBUG(dbgFile, "Can't write to file " << file->filePath() << ", type: " << file->type());
            }
            return result;
        }
        else {
            DEBUG(dbgFile, "Unable to write, file is not open.");
        }
    }
    return -1;
}

int SYS_Close(OpenFileId id) {
    DEBUG(dbgFile, "Closing file " << id);
    if (id <= 1 || id >= MAX_OPEN_FILES) {
        DEBUG(dbgFile, "ID exceeds file descriptor array, id: " << id << ".");
        return -1;
    }
    if (kernel->currentThread->process->closeOpenFile(id))
        return 0;
    return -1;
}
