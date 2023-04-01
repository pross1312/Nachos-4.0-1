// openfile.h 
//	Data structures for opening, closing, reading and writing to 
//	individual files.  The operations supported are similar to
//	the UNIX ones -- type 'man open' to the UNIX prompt.
//
//	There are two implementations.  One is a "STUB" that directly
//	turns the file operations into the underlying UNIX operations.
//	(cf. comment in filesys.h).
//
//	The other is the "real" implementation, that turns these
//	operations into read and write disk sector requests. 
//	In this baseline implementation of the file system, we don't 
//	worry about concurrent accesses to the file system
//	by different threads.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef OPENFILE_H
#define OPENFILE_H

#include "copyright.h"
#include "utility.h"
#include "sysdep.h"
#define FILESYS_STUB

#ifdef FILESYS_STUB			// Temporarily implement calls to 
                    // Nachos file system as calls to UNIX!
                    // See definitions listed under #else
#include "../userprog/syscall.h"
#include <unistd.h>


class OpenFile {
public:
    // open the file
    OpenFile(int f) {
        file = f;
        currentOffset = 0;
        _type = READ_WRITE;
        absolutePath = NULL;
    }

    OpenFile(int f, int t, const char* name) {
        file = f;
        _type = t;
        currentOffset = 0;
        if (t != SOCKET) {
            absolutePath = realpath(name, NULL);
        }
        else
            absolutePath = NULL;
    }
    // close the file
    ~OpenFile() {
        Close(file);
        delete[] absolutePath;
        absolutePath = NULL;
    }

    int ReadAt(char* into, int numBytes, int position) {
        Lseek(file, position, 0);
        return ReadPartial(file, into, numBytes);
    }
    int WriteAt(char* from, int numBytes, int position) {
        Lseek(file, position, 0);
        WriteFile(file, from, numBytes);
        return numBytes;
    }
    int Read(char* into, int numBytes) {
        int numRead = -1;
        if (_type != SOCKET)
            numRead = ReadAt(into, numBytes, currentOffset);
        else
            numRead = read(file, into, numBytes);
        currentOffset += numRead;
        return numRead;
    }
    int Write(char* from, int numBytes) {
        
        int numWritten = -1;
        if (_type == READ_ONLY)
            return -1;
        else if (_type == SOCKET)
            numWritten = write(file, from, numBytes);
        else
            numWritten = WriteAt(from, numBytes, currentOffset);
        currentOffset += numWritten;
        return numWritten;
    }

    int Length() { Lseek(file, 0, 2); return Tell(file); }
    int type() { return _type; }
    const char* filePath() { return absolutePath; }
    int getFileDescriptor() { return file; }
    int Seek(int position) {
        if (_type == SOCKET)
            return -1;
        currentOffset = position;
        return currentOffset;
    }

private:
    int file;
    int currentOffset;
    char* absolutePath;
    int _type;
};

#else // FILESYS
class FileHeader;

class OpenFile {
public:
    OpenFile(int sector);		// Open a file whose header is located
    // at "sector" on the disk
    ~OpenFile();			// Close the file

    void Seek(int position); 		// Set the position from which to 
    // start reading/writing -- UNIX lseek

    int Read(char* into, int numBytes); // Read/write bytes from the file,
    // starting at the implicit position.
    // Return the # actually read/written,
    // and increment position in file.
    int Write(char* from, int numBytes);

    int ReadAt(char* into, int numBytes, int position);
    // Read/write bytes from the file,
// bypassing the implicit position.
    int WriteAt(char* from, int numBytes, int position);

    int Length(); 			// Return the number of bytes in the
    // file (this interface is simpler 
    // than the UNIX idiom -- lseek to 
    // end of file, tell, lseek back 

private:
    FileHeader* hdr;			// Header for this file 
    int seekPosition;			// Current position within the file
};

#endif // FILESYS

#endif // OPENFILE_H
