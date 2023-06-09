#include "../userprog/syscall.h"
#define MAX_ARR 100

int main() {
    int argc = GetArgc();
    char** argv = GetArgv();
    char buffer[MAX_OPEN_FILE_NAME];
    int count_console = 10;
    int count = 10;
    OpenFileId file;
    int i = 0;
    if (argc == 0) {
        if (Write("File name: ", 12, Console_Output) == -1) {
            Exit(2);
        }
        count_console = ConsoleReadLine(buffer, MAX_OPEN_FILE_NAME);
        file = Open(buffer, READ_ONLY);
    }
    else {
        file = Open(argv[1], READ_ONLY);
    }

    count = MAX_ARR;
    while (count == MAX_ARR) {
        char buf[MAX_ARR];
        count = Read(buf, MAX_ARR, file);
        Write(buf, MAX_ARR, Console_Output);

    }
    Write("\n", 1, Console_Output);

    Close(file);

    Exit(0);
}
