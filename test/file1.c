#include "syscall.h"

int main()
{
    while (1) {
        if (Write("1\n", 2, Console_Output) == -1) {
            Halt();
        }
    }

    Halt();
}
