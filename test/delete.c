#include "../userprog/syscall.h"

int main(){
    char buffer[MAX_OPEN_FILE_NAME];
    ConsoleReadLine(buffer,MAX_OPEN_FILE_NAME);
    Remove(buffer);

    Halt();

}
