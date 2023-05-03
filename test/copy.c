#include "../userprog/syscall.h"
#define MAX_ARR 100

// copy from file_src to file_dst
int main() {
    char buffer[MAX_OPEN_FILE_NAME];
    int count = MAX_ARR;
    OpenFileId file_b;
    int tmp = Write("File name src: ", 16, Console_Output);
    int count_console = ConsoleReadLine(buffer, MAX_OPEN_FILE_NAME);
    OpenFileId file_a = Open(buffer, READ_ONLY);

    int mp = Write("File name dst: ", 16, Console_Output);
    int ount_console = ConsoleReadLine(buffer, MAX_OPEN_FILE_NAME);
    if (Write("\n", 1, Console_Output) == -1)
        Exit(3);
    file_b = Open(buffer, READ_WRITE);
    if (file_b == -1) {
        if (Create(buffer) == -1)
            Exit(1);
        file_b = Open(buffer, READ_WRITE);
    }

    while (count == MAX_ARR) {
        char buf[MAX_ARR];
        count = Read(buf, MAX_ARR, file_a);
        count = Write(buf, count, file_b);
    }
    Close(file_a);
    Close(file_b);
    return 0;
}
