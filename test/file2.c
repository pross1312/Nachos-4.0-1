#include "syscall.h"

int main()
{
    int count = 0;
    while (count < 20) {
        Write("File2\n", 6, Console_Output);
        count++;
    }
    Exit(3);
}
