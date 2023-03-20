#include "../userprog/syscall.h"



int main() {
    int id = SocketTCP();
    if (id == -1)
        return 1;

    
    if (Connect(id, "127.0.0.1", 9000) == -1) {
        return 1;
    }


    while (1) {
        char buffer[100];
        int count = ConsoleReadLine(buffer, 100);
        if (Send(id, buffer, count) <= 0)
            break;
    }
    
    SClose(id);
    return 0;
}   
