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

#include "sys_socket.h"
#include "sys_file.h"




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



void ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
    case SyscallException:
        switch (type) {

        

        case SC_Halt: {
            DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
            SysHalt();

            ASSERTNOTREACHED();
            break;
        }

        case SC_Remove: {
            int vAddr = kernel->machine->ReadRegister(4);
            char* fileName = new char[MAX_OPEN_FILE_NAME + 1];
            bzero(fileName, MAX_OPEN_FILE_NAME + 1);
            if (readMemUntil(fileName, vAddr, '\0', MAX_OPEN_FILE_NAME)) {
                kernel->machine->WriteRegister(2, SYS_Remove(fileName));
            }
            else {
                DEBUG(dbgSys, "Read memory error.");
                kernel->machine->WriteRegister(2, -1);
            }
            delete[] fileName;
            return advancePC();
        }

        case SC_Seek: {
            DEBUG(dbgSys, "Move cursor to a position in a file.");
            int position = kernel->machine->ReadRegister(4);
            OpenFileId fileId = kernel->machine->ReadRegister(5);
            kernel->machine->WriteRegister(2, SYS_Seek(position, fileId));
            return advancePC();
        }

        case SC_Create: {
            DEBUG(dbgSys, "Create a file.");
            int virAddr = kernel->machine->ReadRegister(4);
            char* fileName = new char[MAX_OPEN_FILE_NAME + 1];
            bzero(fileName, MAX_OPEN_FILE_NAME + 1);
            if (readMemUntil(fileName, virAddr, '\0', MAX_OPEN_FILE_NAME))
                kernel->machine->WriteRegister(2, SYS_Create(fileName));
            else {
                DEBUG(dbgSys, "Read memory error.");
                kernel->machine->WriteRegister(2, -1);
            }
            delete[] fileName;
            return advancePC();
        }

        case SC_Open: {
            DEBUG(dbgSys, "Open an existing file.");
            char* fileName = new char[MAX_OPEN_FILE_NAME + 1];
            bzero(fileName, MAX_OPEN_FILE_NAME + 1);
            int virAddr = kernel->machine->ReadRegister(4);
            int type = kernel->machine->ReadRegister(5);
            if (readMemUntil(fileName, virAddr, '\0', MAX_OPEN_FILE_NAME))
                kernel->machine->WriteRegister(2, SYS_Open(fileName, type));
            else {
                DEBUG(dbgSys, "Read memory error.");
                kernel->machine->WriteRegister(2, -1);
            }
            delete[] fileName;
            return advancePC();
        }

        case SC_Close: {
            DEBUG(dbgSys, "Close file.");
            OpenFileId fileId = kernel->machine->ReadRegister(4);
            kernel->machine->WriteRegister(2, SYS_Close(fileId));
            return advancePC();
        }


        case SC_Read: {
            DEBUG(dbgSys, "Read from a file");
            int vAddr = kernel->machine->ReadRegister(4);
            int size = kernel->machine->ReadRegister(5);
            OpenFileId fileId = kernel->machine->ReadRegister(6);
            DEBUG(dbgSys, "Sycall read, va " << vAddr << " sz " << size << " id " << fileId);
            char* data = new char[size + 1];
            bzero(data, size + 1);
            int count = SYS_Read(data, size, fileId);
            if (writeToMem(data, size, vAddr))
                kernel->machine->WriteRegister(2, count);
            else
                kernel->machine->WriteRegister(2, -1);
            delete[] data;
            return advancePC();
        }

        case SC_Write: {
            DEBUG(dbgSys, "Write to a file.");
            int vAddr = kernel->machine->ReadRegister(4);
            int size = kernel->machine->ReadRegister(5);
            OpenFileId fileId = kernel->machine->ReadRegister(6);
            DEBUG(dbgSys, "Sycall write, va " << vAddr << " sz " << size << " id " << fileId);
            char* data = new char[size + 1];
            bzero(data, size + 1);
            if (readFromMem(data, size, vAddr)) {
                kernel->machine->WriteRegister(2, SYS_Write(data, size, fileId));
            }
            else {
                DEBUG(dbgSys, "Read memory error.");
                kernel->machine->WriteRegister(2, -1);
            }
            delete[] data;
            return advancePC();
        }

        case SC_ConsoleReadLine: {
            DEBUG(dbgSys, "Read a line from console into a char array.");
            int virAddr = kernel->machine->ReadRegister(4);
            int maxSize = kernel->machine->ReadRegister(5);
            DEBUG(dbgSys, "Sycall read line, va " << virAddr << " sz " << maxSize);
            char* buffer = new char[maxSize + 1];
            bzero(buffer, maxSize + 1);
            int i;
            for (i = 0; i < maxSize; i++) {
                char c = kernel->synchConsoleIn->GetChar();
                if (c == EOF || c == '\n') {
                    buffer[i] = '\0';
                    break;
                }
                buffer[i] = c;
            }
            if (writeToMem(buffer, maxSize, virAddr)) {
                DEBUG(dbgSys, "Successfully read " << i << " bytes.");
                kernel->machine->WriteRegister(2, i);
            }
            else {
                DEBUG(dbgSys, "Write memory error.");
                kernel->machine->WriteRegister(2, -1);
            }
            delete[] buffer;
            return advancePC();
        }

        case SC_SocketTCP: {
            DEBUG(dbgSys, "Create socket.");
            int id = SYS_SocketTCP();
            kernel->machine->WriteRegister(2, id);
            return advancePC();
        }

        case SC_Connect: {
            DEBUG(dbgSys, "Connect to server.");
            int socketID = kernel->machine->ReadRegister(4);
            int vAddr = kernel->machine->ReadRegister(5);
            int port = kernel->machine->ReadRegister(6);
            char* ip = new char[MAX_IP_ADDR_SIZE];
            bzero(ip, MAX_IP_ADDR_SIZE);
            ASSERT(readMemUntil(ip, vAddr, '\0', MAX_IP_ADDR_SIZE));
            int result = SYS_SocketConnect(socketID, ip, port);
            kernel->machine->WriteRegister(2, result);
            delete[] ip;
            return advancePC();
        }
        case SC_Send:
        {
            // DEBUG(dbgSys, "Send message to server.")
            //     int socketID = kernel->machine->ReadRegister(4);
            // int vAddr = kernel->machine->ReadRegister(5);
            // int len = kernel->machine->ReadRegister(6);
            // char* buffer = new char[len];
            // bzero(buffer, len);

            // ASSERT(readFromMem(buffer, len, vAddr))
            //     int result = SYS_SocketSend(socketID, buffer, len);
            // kernel->machine->WriteRegister(2, result);

            // delete[] buffer;
            // return advancePC();
        }

        case SC_SReceive:
        {
            // int socketID = kernel->machine->ReadRegister(4);
            // int vAddr = kernel->machine->ReadRegister(5);
            // int len = kernel->machine->ReadRegister(6);
            // char* buffer = new char[len];
            // int result = SYS_SocketReceive(socketID, buffer, len);
            // kernel->machine->WriteRegister(2, result);

            // ASSERT(writeToMem(buffer, len, vAddr));
            // delete[] buffer;
            // return advancePC();
        }


        case SC_SClose:
        {
            // int socketID = kernel->machine->ReadRegister(4);
            // int status = SYS_SocketClose(socketID);
            // if (status < 0)
            //     kernel->machine->WriteRegister(2, -1);
            // else
            //     kernel->machine->WriteRegister(2, 0);
            // return advancePC();
        }



        case SC_Add: {
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
        }
        case SC_Exec: {
            int vAddr = kernel->machine->ReadRegister(4);
            char* name = new char[MAX_OPEN_FILE_NAME + 1];
            bzero(name, MAX_OPEN_FILE_NAME + 1);
            readMemUntil(name, vAddr, '\0', MAX_OPEN_FILE_NAME);
            AddrSpace* space = new AddrSpace;
            if (space->Load(name)) {
                Thread* t = new Thread(name);
                t->space = space;
                Process* p= Process::createProcess(kernel->currentThread->process, t, name);
                int id = -1;
                if(p!=NULL){
                    id=p->getId();
                    DEBUG(dbgSys, "Create process " <<name<< " with ID: " << id  << endl);
                }
                kernel->machine->WriteRegister(2, id);
            }else{
                DEBUG(dbgSys, "Can't load AddrSpace\n");
                kernel->machine->WriteRegister(2, -1);
            }
            delete[] name;
            return advancePC();
        }
        case SC_Join: {
            int iD = kernel->machine->ReadRegister(4);
            if (iD<0 || iD >=MAX_RUNNING_PROCESS){
                DEBUG(dbgSys, "Out of allowed quantity\n");
                kernel->machine->WriteRegister(2, -1);
                return advancePC();
            }
            if(!kernel->pTable->checkID(iD)){
                DEBUG(dbgSys, "No process\n");
                kernel->machine->WriteRegister(2, -1);
                return advancePC();
            }
            SpaceId processID = -1;
            for (int i=0;i<MAX_RUNNING_PROCESS;i++){
                if(!kernel->pTable->checkID(i)) continue;
                if(kernel->pTable->get(i)->getThread() == kernel->currentThread){
                    processID =i;
                    break;
                }
            }
            if (processID == -1) {
                DEBUG(dbgSys, "Has no parent\n");
                kernel->machine->WriteRegister(2, -1);
                return advancePC();
            }
            if (kernel->pTable->get(iD)->getParent()->getId() != processID) {
                DEBUG(dbgSys, "Has no parent\n");
                kernel->machine->WriteRegister(2, -1);
                return advancePC();
            }
            kernel->pTable->get(processID)->JoinWait(iD);
            int exitCode = kernel->pTable->get(processID)->getExitCode();
            kernel->machine->WriteRegister(2, exitCode);
            return advancePC();
        }
        case SC_Exit: {
            int exitCode = kernel->machine->ReadRegister(4);
            int processID = -1;
            for (int i = 0; i < MAX_RUNNING_PROCESS; ++i) {
                if(!kernel->pTable->checkID(i)) continue;
                if(kernel->pTable->get(i)->getThread() == kernel->currentThread){
                    processID =i;
                    break;
                }
            }
            if (processID == -1) {
                DEBUG(dbgSys, "Has no parent\n");
                kernel->machine->WriteRegister(2, -1);
                return advancePC();
            }
            Process* parent = kernel->pTable->get(processID)->getParent();
            kernel->pTable->get(processID)->ExitWait();

            if (parent == NULL) {
                kernel->interrupt->Halt();
            } else {
                int parentID = parent->getId();
                kernel->pTable->get(processID)->JoinRelease(processID, exitCode);
                kernel->pTable->get(processID)->DecNumWait();
                kernel->pTable->get(processID)->ExitRelease();

                kernel->pTable->remove(processID);
                kernel->currentThread->Finish();
                kernel->machine->WriteRegister(2, 0);
                return advancePC();
            }



            // kernel->machine->WriteRegister(2, -1);
            // return advancePC();
        }
        default:
            cerr << "Unexpected system call " << type << "\n";
            break;
        }
        break;
    default:
        cerr << "Unexpected user mode exception " << (int)which << "\n";
        break;
    }
    ASSERTNOTREACHED();
}
