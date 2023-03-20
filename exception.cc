// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "synchconsole.h"
#include "syscall.h"
#include "ksyscall.h"
#include "helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <errno.h>


#define Size 100




//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------
#define ssize 100
const int MAX_OPEN_FILES = 20;
static pair<OpenFile*, int> FileDescriptor[MAX_OPEN_FILES];
int *OpenSocketID = (int*)malloc(20*sizeof(int));
void ExceptionHandler(ExceptionType which) {
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
    case SyscallException:
        switch (type) {

        case SC_Exit: {
            DEBUG(dbgSys, "Exit program...");
            int status = kernel->machine->ReadRegister(4);
            for (int i = 2; i < MAX_OPEN_FILES; i++)
                if (FileDescriptor[i].first != NULL) 
                    delete FileDescriptor[i].first;
            if (status == 0)
                cout << endl << "Exit normal. " << endl;
            else
                cout << "Exit with an error code " << status << endl;
            exit(status);
            ASSERTNOTREACHED();
        }

        case SC_Halt: {
            DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
            for (int i = 2; i < MAX_OPEN_FILES; i++)
                if (FileDescriptor[i].first != NULL)
                    delete FileDescriptor[i].first;
            SysHalt();

            ASSERTNOTREACHED();
            break;
        }

        case SC_Remove: {
            int vAddr = kernel->machine->ReadRegister(4);
            static char fileName[MAX_OPEN_FILE_NAME];
            bzero(fileName, MAX_OPEN_FILE_NAME);
            ASSERT(readMemUntil(fileName, vAddr, '\0', MAX_OPEN_FILE_NAME));
            if (kernel->fileSystem->Remove(fileName))
                kernel->machine->WriteRegister(2, 0);
            else
                kernel->machine->WriteRegister(2, -1);
        
            return advancePC();
        }

        case SC_Seek: {
            DEBUG(dbgSys, "Move cursor to a position in a file.");
            int position = kernel->machine->ReadRegister(4);
            OpenFileId fileId = kernel->machine->ReadRegister(5);
            ASSERT(fileId > 1 && fileId < MAX_OPEN_FILES);
            OpenFile* file = FileDescriptor[fileId].first;

            int length = file->Length();
            DEBUG(dbgSys, "Length file: " << length << " seek position: " << position);
            if (position > length || position < -1)
                kernel->machine->WriteRegister(2, -1);
            else {
                if (position == -1)
                    position = length;
                file->Seek(position);
                kernel->machine->WriteRegister(2, position);
            }
            return advancePC();
        }

        case SC_Create: {
            DEBUG(dbgSys, "Create a file.");
            static char fileName[MAX_OPEN_FILE_NAME];
            bzero(fileName, MAX_OPEN_FILE_NAME);
            int virAddr = kernel->machine->ReadRegister(4);
            readMemUntil(fileName, virAddr, '\0', MAX_OPEN_FILE_NAME);
#ifdef FILESYS_STUB
            if (kernel->fileSystem->Create(fileName)) {
                kernel->machine->WriteRegister(2, 0);
                DEBUG(dbgSys, "Create file successfully." << fileName);
            }
            else {
                DEBUG(dbgSys, "Can't create file." << fileName);
                kernel->machine->WriteRegister(2, -1);
        }
#else
            if (kernel->fileSystem->Create(fileName, 0))
                kernel->machine->WriteRegister(2, 0);
            else
                kernel->machine->WriteRegister(2, -1);
#endif
            return advancePC();
        }

        case SC_Open: {
            DEBUG(dbgSys, "Open an existing file.");
            static char fileName[MAX_OPEN_FILE_NAME];
            bzero(fileName, MAX_OPEN_FILE_NAME);
            int virAddr = kernel->machine->ReadRegister(4);
            int type = kernel->machine->ReadRegister(5);
            

            if (readMemUntil(fileName, virAddr, '\0', MAX_OPEN_FILE_NAME)) {
                DEBUG(dbgSys, "Opening file " << fileName << "...");
                // FILE* file = fopen(fileName, "r+");
                OpenFile* file = kernel->fileSystem->Open(fileName);
                if (file) {
                    // start from 2 as 0 and 1 are for console input and output
                    for (int i = 2; i < MAX_OPEN_FILES; i++)
                        if (FileDescriptor[i].first == NULL) {
                            FileDescriptor[i].first = file;
                            FileDescriptor[i].second = type;
                            kernel->machine->WriteRegister(2, i);
                            DEBUG(dbgSys, "Open successfully file " << fileName << " --- id: " << i);
                            return advancePC();
                        }
                    cerr << "System error: no available space to open file." << endl;
                    return advancePC();
                }
            }
            cout << "Fail to open file " << fileName << "." << endl;
            kernel->machine->WriteRegister(2, -1);
            return advancePC();
        }

        case SC_Close: {
            DEBUG(dbgSys, "Close file.");
            OpenFileId fileId = kernel->machine->ReadRegister(4);
            ASSERT(fileId > 1 && fileId <= MAX_OPEN_FILES);

            OpenFile* file = FileDescriptor[fileId].first;

            if (!file)
                cerr << "File is not opening." << endl;
            else {
                DEBUG(dbgSys, "Successfully close file id: " << fileId);
                delete file;
                FileDescriptor[fileId].first = NULL;
                FileDescriptor[fileId].second = -1;
            }

            return advancePC();
        }


        case SC_Read: {
            DEBUG(dbgSys, "Read from a file");
            int vAddr = kernel->machine->ReadRegister(4);
            int size = kernel->machine->ReadRegister(5);
            OpenFileId fileId = kernel->machine->ReadRegister(6);

            ASSERT(fileId == Console_Input || (fileId > 1 && fileId < MAX_OPEN_FILES));

            char* data = new char[size];
            bzero(data, size);
            if (fileId == 0) {
                char ch;
                int i;
                for (i = 0; i < size; i++) {
                    ch = kernel->synchConsoleIn->GetChar();
                    if (ch == EOF)
                        break;
                    data[i] = ch;
                }
                kernel->machine->WriteRegister(2, i);
            }
            else {
                OpenFile* file = FileDescriptor[fileId].first;
                if (file) {
                    int count = file->Read(data, size);
                    DEBUG(dbgSys, "Read " << count << " bytes from file " << fileId);
                    kernel->machine->WriteRegister(2, count);
                }
                else
                    kernel->machine->WriteRegister(2, -1);
            }
            writeToMem(data, size, vAddr);
            delete[] data;
            return advancePC();
        }

        case SC_Write: {
            DEBUG(dbgSys, "Write to a file.");
            int vAddr = kernel->machine->ReadRegister(4);
            int size = kernel->machine->ReadRegister(5);
            OpenFileId fileId = kernel->machine->ReadRegister(6);


            ASSERT(fileId == 1 || (fileId > 1 && fileId < MAX_OPEN_FILES));



            char* data = new char[size];
            bzero(data, size);
            ASSERT(readFromMem(data, size, vAddr));

            if (fileId == 1) {
                DEBUG(dbgSys, "Write " << data << " to console.");
                for (int i = 0; i < size; i++)
                    kernel->synchConsoleOut->PutChar(data[i]);
                kernel->machine->WriteRegister(2, size);
            }
            else {
                DEBUG(dbgSys, "Write " << data << " to file " << fileId);
                OpenFile* file = FileDescriptor[fileId].first;
                if (file && FileDescriptor[fileId].second == READ_WRITE) {
                    int result = file->Write(data, size);
                    DEBUG(dbgSys, "Successfully write " << result << " bytes");
                    kernel->machine->WriteRegister(2, result);
                }
                else {
                    if (file) {
                        DEBUG(dbgSys, "Unable to write, file is read-only.");
                    }
                    else {
                        DEBUG(dbgSys, "Unable to write, file is not open.");
                    }
                    kernel->machine->WriteRegister(2, -1);
                }
            }
            delete[] data;
            return advancePC();
        }
        // case SC_ConsoleReadLine: {
        //     DEBUG(dbgSys, "Read a line from console into a char array.");
        //     int virAddr = kernel->machine->ReadRegister(4);
        //     int maxSize = kernel->machine->ReadRegister(5);
        //     char* temp = new char[maxSize] {};
        //     // cin.getline(temp, maxSize, '\n');
        //     int i;
        //     for (i = 0; i < maxSize; i++) {
        //         char c = kernel->synchConsoleIn->GetChar();
        //         if (c == EOF || c == '\n')
        //             break;
        //         temp[i] = c;
        //     }
        //     if (writeToMem(temp, strlen(temp), virAddr))
        //         kernel->machine->WriteRegister(2, i);
        //     else
        //         kernel->machine->WriteRegister(2, -1);
        //     delete[] temp;
        //     return advancePC();
        // }
        // case SC_ConsoleWrite: {
        //     DEBUG(dbgSys, "Write char array to console.");
        //     int virAddr = kernel->machine->ReadRegister(4);
        //     int size = kernel->machine->ReadRegister(5);
        //     char* temp = new char[size] {};
        //     if (readFromMem(temp, size, virAddr)) {
        //         cout.write(temp, size);
        //         kernel->machine->WriteRegister(2, 0);
        //     }
        //     else
        //         kernel->machine->WriteRegister(2, -1);
        //     delete[] temp;
        //     return advancePC();
        // }
        // case SC_ConsoleRead: {
        //     DEBUG(dbgSys, "Read input to char array.\n");
        //     int buffer = kernel->machine->ReadRegister(4);
        //     int size = kernel->machine->ReadRegister(5);
        //     char* temp = new char[size];
        //     bzero(temp, size);
        //     int count = fread(temp, 1, size, stdin);
        //     cin.get();
        //     if (writeToMem(temp, size, buffer)) {
        //         kernel->machine->WriteRegister(2, count);
        //     }
        //     else
        //         kernel->machine->WriteRegister(2, -1);

        //     delete[] temp;
        //     return advancePC();
        // }
        // case SC_ConsoleWriteLine: {
        //     DEBUG(dbgSys, "Write char array on a line.");
        //     int vAddr = kernel->machine->ReadRegister(4);
        //     int size = kernel->machine->ReadRegister(5);
        //     char* temp = new char[size];
        //     if (readFromMem(temp, size, vAddr)) {
        //         int count = fwrite(temp, 1, size, stdout);
        //         cout << endl;
        //         kernel->machine->WriteRegister(2, count + 1);
        //     }
        //     else
        //         kernel->machine->WriteRegister(2, -1);
        //     delete[] temp;
        //     return advancePC();
        // }

        case SC_SocketTCP:
        {
            int sock;
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0)
                kernel->machine->WriteRegister(2, -1);
            else {
                for (int i = 0; i < 20; i++)
                {
                    if (OpenSocketID[i] == 0)
                    {
                        OpenSocketID[i] = sock;
                        kernel->machine->WriteRegister(2, i);
                    }
                }
            }
            return advancePC();
        }

        // int Connect(int socketID, char* ip, int port)
        case SC_Connect:
        {
            // SocketId Id = kernel->machine->ReadRegister(4);
            // int port = kernel->machine->ReadRegister(5);
            int socketID = kernel->machine->ReadRegister(4);
            int vAddr = kernel->machine->ReadRegister(5);
            int port = kernel->machine->ReadRegister(6);
            char *ip = (char*)malloc(4*sizeof(char));
            bzero(ip, 4);
            if (!readFromMem(ip, 4, vAddr))
            {
                // fail
            }

            // int client_socket = OpenSocket[socketID];
            struct sockaddr_in server_addr;

            //  server_socket = socket(AF_INET, SOCK_STREAM, 0);
            server_addr.sin_port = htons(port);
            server_addr.sin_addr.s_addr = inet_addr(ip);

            if (connect(socketID, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
                kernel->machine->WriteRegister(2, -1);
            else
                kernel->machine->WriteRegister(2, 0);
            free(ip);
            return advancePC();
        }
    case SC_Send:
    {
        int socketID = kernel->machine->ReadRegister(4);
        int vAddr = kernel->machine->ReadRegister(5);
        int len = kernel->machine->ReadRegister(6);
        char *buffer = (char*)malloc(len);
        bzero(buffer, len);

        if (!readFromMem(buffer, len, vAddr))
            {
                // fail
            }

        int n;
        n = write(socketID, buffer, len);
        if (n < 0)
            kernel->machine->WriteRegister(2, -1);
        else if (n == 0)
            kernel->machine->WriteRegister(2, 0);
        else
            kernel->machine->WriteRegister(2, n);

        free(buffer);
        return advancePC();
    }
    case SC_SReceive:
    {   
        int socketID = kernel->machine->ReadRegister(4);
        int vAddr = kernel->machine->ReadRegister(5);
        int len = kernel->machine->ReadRegister(6);
        char *buffer = (char*)malloc(len);
        bzero(buffer, len);

        if (!readFromMem(buffer, len, vAddr))
            {
                // fail
            }
        int n = read(socketID, buffer, Size);
        if (n < 0)
            kernel->machine->WriteRegister(2, -1);
        else if (n == 0)
            kernel->machine->WriteRegister(2, 0);
        else
            kernel->machine->WriteRegister(2, n);


        free(buffer);
        return advancePC();
    }
    case SC_SClose:
    {   
        int socketID = kernel->machine->ReadRegister(4);
        int n = close(socketID);
        if (n < 0)
            kernel->machine->WriteRegister(2, -1);
        else
            kernel->machine->WriteRegister(2, 0);

        return advancePC();
    }
        case SC_Add:
            DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

            /* Process SysAdd Systemcall*/
            int result;
            result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
                /* int op2 */(int)kernel->machine->ReadRegister(5));
            DEBUG(dbgSys, "Add returning with " << result << "\n");
            /* Prepare Result */
            kernel->machine->WriteRegister(2, (int)result);

            /* Modify return point */
            {
                /* set previous programm counter (debugging only)*/
                kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

                /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

                /* set next programm counter for brach execution */
                kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
            }

            return;

            ASSERTNOTREACHED();

            break;

        default:
            cerr << "Unexpected system call " << type << "\n";
            break;
    }
        break;
    default:
        cerr << "Unexpected user mode exception" << (int)which << "\n";
        break;
}


    // ASSERTNOTREACHED();
}