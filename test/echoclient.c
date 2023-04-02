#include "../userprog/syscall.h"
#define MAX_IP 15
#define PORT 8080
#define BUFFER_SIZE 1024
#define client "Client: "
int main() {
    int sock[4];
    int tmp = 0;
    char ip[MAX_IP];
    int i = 0;

    for (; i < 4;i++) {
        sock[i] = SocketTCP();
        if (sock[i] == -1) {
            Exit(1);
        }
    }

    Write("Enter Ip: ", 11, Console_Output);
    ConsoleReadLine(ip, MAX_IP);

    for (i = 0; i < 4;i++) {
        if (Connect(sock[i], ip, PORT) == -1) {
            Exit(1);
        }
    }

    for (i = 0; i < 4;i++) {
        while (1) {
            char buffer[BUFFER_SIZE];
            int count_w = 1;
            Write(client, 8, Console_Output);
            tmp = ConsoleReadLine(buffer, BUFFER_SIZE);
            if ((count_w = Write(buffer, tmp, sock[i])) == 0)
                break;
            Write("Server: ", 9, Console_Output);
            Write(buffer, Read(buffer, BUFFER_SIZE, sock[i]), Console_Output);
            Write("\n", 2, Console_Output);
        }
        Close(sock[i]);
    }
    Halt();
}
