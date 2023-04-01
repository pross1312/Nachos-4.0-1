#include "../userprog/syscall.h"

int main() {
    OpenFileId file_input_id = 0;
    OpenFileId file_output_id = 0;
    int sockID = 0;
    char fileName[MAX_OPEN_FILE_NAME];
    char sendBuff[100];
    char readBuff[100];
    char ip[20];
    int check = 0;

    check = Write("Input file: ", 13, Console_Output);
    if (check == -1) {
        Exit(10);
    }

    check = ConsoleReadLine(fileName, MAX_OPEN_FILE_NAME);
    if (check <= 0) {
        Exit(1);
    }

    file_input_id = Open(fileName, READ_ONLY);

    if (file_input_id == -1) {
        Exit(2);
    }

    check = Write("Output file: ", 14, Console_Output);
    if (check == -1) {
        Exit(11);
    }



    check = ConsoleReadLine(fileName, MAX_OPEN_FILE_NAME);
    if (check <= 0) {
        Exit(3);
    }

    file_output_id = Open(fileName, READ_WRITE);
    if (file_output_id == -1) {
        check = Create(fileName);
        if (check == -1) {
            Exit(4);
        }
        file_output_id = Open(fileName, READ_WRITE);
        if (file_output_id == -1) {
            Exit(4);
        }
    }

    sockID = SocketTCP();
    if (sockID == -1) {
        Exit(5);
    }

    check = Write("Ip: ", 5, Console_Output);
    if (check == -1) {
        Exit(12);
    }

    check = ConsoleReadLine(ip, 20);
    if (check <= 0) {
        Exit(13);
    }

    check = Connect(sockID, ip, 8080);
    if (check == -1) {
        Exit(6);
    }

    do {
        check = Read(sendBuff, 100, file_input_id);
        if (Write(sendBuff, check, sockID) == -1) {
            Exit(19);
        }
        check = Read(readBuff, 100, sockID);
        if (check == -1) {
            Exit(20);
        }
        Write(readBuff, check, file_output_id);
    } while (check == 100);

    Halt();
}
