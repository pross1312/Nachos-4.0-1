#include "syscall.h"

int main()
{
    int count = 0;
    int fileid = Open("Output.txt", READ_WRITE);
    SpaceId i = Exec("file1");
    Join(i);
    while (count < 1000) {
        Write("file0\n", 6, fileid);
        count++;
    }
    Exit(0);
}

