#include "../userprog/syscall.h"
#define MAX_ARR 100

int main(){
    char buffer[MAX_OPEN_FILE_NAME];
    int tmp = Write("Name file a\n",12,Console_Output);
    int count_console = ConsoleReadLine(buffer,MAX_OPEN_FILE_NAME);
    OpenFileId file_a = Open(buffer, READ_ONLY);
    
    int mp =  Write("Name file b\n",12,Console_Output);
    int     ount_console = ConsoleReadLine(buffer,MAX_OPEN_FILE_NAME);
    
    OpenFileId file_b = Open(buffer, READ_WRITE);
    int tmp_2 = Seek(-1,file_b);
    // if (file_b ==-1) {
    //     int tmp_1  = Create(buffer);
    //     file_b = Open(buffer, READ_WRITE);
    // }


    int count =MAX_ARR;
    while (count == MAX_ARR) {
        char buf[MAX_ARR];
        count = Read(buf,MAX_ARR,file_a);
        count = Write(buf,count,file_b);
    }
    Close(file_a);
    Close(file_b);
    Halt();
    Halt();

}
