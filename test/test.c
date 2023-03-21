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
        if (Write(buffer, count, id) <= 0)
            break;
    }
    
    Close(id);
    return;
}   
