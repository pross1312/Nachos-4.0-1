#include "syscall.h"

int main()
{
    int count = 0;
    OpenFileId f = Open("output.txt", READ_WRITE);
    while (count < 20) {
        Write("File2\n", 6, f);
        count++;
    }
    Exit(3);
}
