/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "../userprog/syscall.h"

int main(){
    int fileId, count;
    char buffer[20];
    Create("Hello.txt");
    fileId = Open("Hello.txt", READ_WRITE);
    count = Write("My name is tuong\n", 18, fileId);
    if (count != -1) {
        if (Seek(0, fileId) == -1)
            Exit(1);
        count = Read(buffer, count, fileId);
        if (count != -1) {
            count = Write(buffer, count, Console_Output);
        }
    }
    Close(fileId);
    return 0;
}
