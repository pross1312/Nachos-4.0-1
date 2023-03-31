#include "../userprog/syscall.h"
#define MAX_IP 15
#define PORT 8080
#define BUFFER_SIZE 1024

int main(){
    int count_w=BUFFER_SIZE;
    int sock=SocketTCP();
    int tmp=0;
    char ip[MAX_IP];
    int i=0;
    
    char file_name[MAX_OPEN_FILE_NAME];
    OpenFileId file,file_tmp;
    Write("Name file\n",10,Console_Output);
    ConsoleReadLine(file_name,MAX_OPEN_FILE_NAME);

    file = Open(file_name, READ_WRITE);
    if(Create("t.txt")){
        Exit(1);
    }
    file_tmp = Open("t.txt",READ_WRITE);

    if(file==-1)
        Exit(1);

    if(sock ==-1){
        Exit(1);
    }
    
    Write("Enter Ip:\n",10,Console_Output);
    ConsoleReadLine(ip, MAX_IP);

    if(Connect(sock,ip,PORT)==-1){
        Exit(1);
    }
    while(count_w== BUFFER_SIZE){
        char buffer[BUFFER_SIZE];
        int count_w= Read(buffer,BUFFER_SIZE,file);
        if((tmp=Write(buffer,count_w,sock)) == 0)
            break;
        Write(buffer,Read(buffer,BUFFER_SIZE,sock),file_tmp);
    }
    while(count_w== BUFFER_SIZE){
        char buf[BUFFER_SIZE];
        count_w = Read(buf,BUFFER_SIZE,file_tmp);
        count_w = Write(buf,count_w,file);
    }
    Close(sock);
    Close(file);
    Close(file_tmp);
    Remove("t.txt");
    Halt();
}
