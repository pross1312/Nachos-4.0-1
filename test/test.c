#include "../userprog/syscall.h"



int main() {
    int id = SocketTCP();
    if (id == -1)
        Exit(1);    
    if (Connect(id, "127.0.0.1", 9000) == -1) {
        return 1;
    }


    while (1) {
        char buffer[100];
        int count = ConsoleReadLine(buffer, 100);
        if (count <= 0) {
            Close(id);
            Write("Close connection.\n", 18, Console_Output);
            break;
        }
        Write(buffer, count, id);
        count = Read(buffer, 99, id);
        Write("Sever sent: ", 12, Console_Output);
        Write(buffer, count, Console_Output);
    }
    
    Close(id);
    return;
}   
