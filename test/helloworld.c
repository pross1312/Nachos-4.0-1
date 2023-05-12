#include "syscall.h"

int main()
{
    while (1) {
        Write("Hello world\n", 12, Console_Output);
    }
    Halt();
}
