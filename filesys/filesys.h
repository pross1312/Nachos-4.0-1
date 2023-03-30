
// filesys.h 
//	Data structures to represent the Nachos file system.
//
//	A file system is a set of files stored on disk, organized
//	into directories.  Operations on the file system have to
//	do with "naming" -- creating, opening, and deleting files,
//	given a textual file name.  Operations on an individual
//	"open" file (read, write, close) are to be found in the OpenFile
//	class (openfile.h).
//
//	We define two separate implementations of the file system. 
//	The "STUB" version just re-defines the Nachos file system 
//	operations as operations on the native UNIX file system on the machine
//	running the Nachos simulation.
//
//	The other version is a "real" file system, built on top of 
//	a disk simulator.  The disk is simulated using the native UNIX 
//	file system (in a file named "DISK"). 
//
//	In the "real" implementation, there are two key data structures used 
//	in the file system.  There is a single "root" directory, listing
//	all of the files in the file system; unlike UNIX, the baseline
//	system does not provide a hierarchical directory structure.  
//	In addition, there is a bitmap for allocating
//	disk sectors.  Both the root directory and the bitmap are themselves
//	stored as files in the Nachos file system -- this causes an interesting
//	bootstrap problem when the simulated disk is initialized. 
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef FS_H
#define FS_H

#include "copyright.h"
#include "sysdep.h"
#include "openfile.h"
#include "debug.h"

#ifdef FILESYS_STUB 		// Temporarily implement file system calls as 
                // calls to UNIX, until the real file system
                // implementation is available
const int MAX_OPEN_FILES = 20;
class FileSystem {
public:
    FileSystem() {
        for (int i = 0; i < MAX_OPEN_FILES; i++)
            FileDescriptor[i] = NULL;
    }
    ~FileSystem() {
        for (int i = 0; i < MAX_OPEN_FILES; i++)
            delete FileDescriptor[i];
    }

    bool Create(char* name) {
        int fileDescriptor = OpenForWrite(name);
        if (fileDescriptor == -1) return FALSE;
        Close(fileDescriptor);
        return TRUE;
    }

    // find empty slot and store openfile
    int add(OpenFile* f) {
        // 0 and 1 for console input/output
        for (int i = 2; i < MAX_OPEN_FILES; i++)
            if (FileDescriptor[i] == NULL) {
                DEBUG(dbgFile, "Add open file successfully: slot " << i);
                FileDescriptor[i] = f;
                return i;
            }
        DEBUG(dbgFile, "Add open file error: no available space.");
        return -1;
    }

    bool Close(int id) {
        OpenFile* file = FileDescriptor[id];
        if (!file) {
            DEBUG(dbgFile, "File is not openning id: " << id);
            return false;
        }
        DEBUG(dbgFile, "Successfully close file id: " << id);
        FileDescriptor[id] = NULL;
        delete file;
        return true;
    }

    // method for open file syscall....
    int Open(char* name, int t) {
        if (t == SOCKET) {
            DEBUG(dbgFile, "Can't open file with socket type.");
            return NULL;
        }
        int fileDescriptor = OpenForReadWrite(name, FALSE);

        if (fileDescriptor == -1) {
            DEBUG(dbgFile, "Unable to open file " << name);
            return -1;
        }
        OpenFile* file = new OpenFile(fileDescriptor, t, name);
        int result = add(file);
        if (result != -1) {
            DEBUG(dbgFile, "Open file " << name << " type " << t << " successfully.");
        }
        return result;
    }

    bool isOpen(char* name) {
        char* full_path = realpath(name, NULL);
        if (full_path == NULL) {
            DEBUG(dbgFile, "Check open file error: can't find full path.");
            return false;
        }
        for (int i = 2; i < MAX_OPEN_FILES; i++)
            if (FileDescriptor[i] != NULL && strcmp(full_path, FileDescriptor[i]->filePath()) == 0) {
                DEBUG(dbgFile, "File " << name << " is open.")
                    return true;
            }
        DEBUG(dbgFile, "File " << name << " isn't open.");
        return false;
    }

    OpenFile* Open(char* name) {
        int fileDescriptor = OpenForReadWrite(name, FALSE);

        if (fileDescriptor == -1) return NULL;
        return new OpenFile(fileDescriptor);
    }

    OpenFile* get(int id) {
        ASSERT(id < MAX_OPEN_FILES && id > 1);
        return FileDescriptor[id];
    }

    bool Remove(char* name) {
        if (isOpen(name)) {
            DEBUG(dbgFile, "Can't remove open file.")
                return false;
        }
        return Unlink(name) == 0;
    }

private:
    OpenFile* FileDescriptor[MAX_OPEN_FILES];
};

#else // FILESYS


class FileSystem {
public:
    FileSystem(bool format);		// Initialize the file system.
    // Must be called *after* "synchDisk" 
    // has been initialized.
        // If "format", there is nothing on
    // the disk, so initialize the directory
        // and the bitmap of free blocks.

    bool Create(char* name, int initialSize);
    // Create a file (UNIX creat)

    OpenFile* Open(char* name); 	// Open a file (UNIX open)

    bool Remove(char* name);  		// Delete a file (UNIX unlink)

    void List();			// List all the files in the file system

    void Print();			// List all the files and their contents

private:
    OpenFile* freeMapFile;		// Bit map of free disk blocks,
    // represented as a file
    OpenFile* directoryFile;		// "Root" directory -- list of 
    // file names, represented as a file
};

#endif // FILESYS

#endif // FS_H
