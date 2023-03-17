#include "../userprog/syscall.h"



int main() {
    int id = SocketTCP();

    char ip[] = "127.0.0.1";
    int c = Connect(id, ip, 9000);
    Send(id, "Hello world\n", 12);
    // Write("Hellorld\n", 13, Console_Output);
    return 0;
}   
