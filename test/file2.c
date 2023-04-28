#include "syscall.h"

int main()
{
    int fileID = 0;
    int count = 0;
    if (Create("output.txt") == -1) {
        Halt();
    }
    fileID = Open("output.txt", READ_WRITE);
    if (fileID == -1) {
        Halt();
    }
    while (count < 10) {
        if (Write("Hello wolrd\n", 13, fileID) == -1) {
            Halt();
        }
        count++;
    }
    Halt();
}
