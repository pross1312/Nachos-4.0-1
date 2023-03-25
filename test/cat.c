#include "../userprog/syscall.h"
#define MAX_ARR 100

int main(){
    char buffer[MAX_OPEN_FILE_NAME];
    int count_console = ConsoleReadLine(buffer,MAX_OPEN_FILE_NAME);
    OpenFileId file = Open(buffer, READ_ONLY);

    int count = MAX_ARR;
    while(count == MAX_ARR){
        char buf[MAX_ARR];
        count = Read(buf,MAX_ARR,file);
        Write(buf,MAX_ARR,Console_Output);
        
    }
    Write("\n",1,Console_Output);

    Close(file);

    Halt();
}
