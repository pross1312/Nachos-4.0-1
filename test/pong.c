#include "syscall.h"

int main() {
    int count = 0;
    while (count < 10) {
        Wait("console_out");
        Write("Pong\n", 5, Console_Output);
        Signal("console_out");
        count++;
    }
    Exit(0);
}
