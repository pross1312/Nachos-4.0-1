#include "syscall.h"

int main()
{
    Exec("file1");
    Exec("file2");
    while (1) {}
    Halt();
}
