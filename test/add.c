/* add.c
 *	Simple program to test whether the systemcall interface works.
 *
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "../userprog/syscall.h"

int main() {
    int count, id;
    Create("a.txt");
    id = Open("a.txt", READ_WRITE);
    if (id == -1)
        Exit(1);
    if (Remove("a.txt") == -1) {
        count = Write("Can't remove.\n", 15, Console_Output);
    }
    Close(id);
    if (Remove("a.txt") == 0) {
        count = Write("Remove successfully\n", 21, Console_Output);
    }
    return;
}
