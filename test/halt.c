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
    if (Create("Hello.txt") == 0) {
        int id = Open("Hello.txt", READ_WRITE);
        int count = Write("Toi la tuong\n", 14, id);
        if (count != -1) {
            char buffer[20];
            int s = Seek(3, id);
            int read_count = Read(buffer, count, id);
            if (read_count != -1) {
                int check = Write(buffer, read_count, Console_Output);
            }
        }
        
        Close(id);
        Remove("Hello.txt");
    }

    return 0;
}
