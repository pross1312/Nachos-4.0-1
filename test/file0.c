#include "syscall.h"

int main()
{
    int count = 0;
    SpaceId i = Exec("file1");
    
    Join(i);
    while (count < 20) {
        Write("file0\n", 6, Console_Output);
        count++;
    }
    Exit(0);
}

