#include "syscall.h"

int main() {
    int argc = GetArgc();
    char** argv = GetArgv();
    int count = 0;
    int size = SizeOf(argv[1]);
    if (argc != 2)
        Exit(1);
    while (count < 10) {
        Wait("console_out");

        Write(argv[1], size, Console_Output);
        Write("\n", 1, Console_Output);

        Signal("console_out");
        count++;
    }
    

    Exit(0);
}
