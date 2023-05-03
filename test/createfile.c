#include "../userprog/syscall.h"

int main() {
    char buffer[MAX_OPEN_FILE_NAME];
    int count = 0;
    if (Write("File name: ", 12, Console_Output) == -1) {
        Exit(1);
    }
    count = ConsoleReadLine(buffer, MAX_OPEN_FILE_NAME);
    if (Create(buffer) == -1) {
        Write("Can't create file ", 19, Console_Output);
        Write(buffer, count, Console_Output);
    }
    Exit(0);
}
