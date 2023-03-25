#include "../userprog/syscall.h"

int main(){
    char buffer[MAX_OPEN_FILE_NAME];
    int count = ConsoleReadLine(buffer,MAX_OPEN_FILE_NAME);
    int yn = Create(buffer);
    Halt();
}
