#include "../userprog/syscall.h"
#define MAX_IP 15
#define MAX_PORT 10
#define MAX_IB 100
#define PORT 8080
int main(){
    int sock[4];
    int tmp=0;
    char ip[MAX_IP];
    int i=0;

    for (; i<4;i++){
        sock[i]=SocketTCP();
        if(sock[i] ==-1){
            Exit(1);
        }
    }

    Write("Enter Ip: ",10,Console_Output);
    ConsoleReadLine(ip, MAX_IP);

    for(i=0;i<4;i++){
        if(Connect(sock[i],ip,PORT) == -1){
            Exit(1);
        }
        while(1){
            char buffer[MAX_IB];
            int count_w=1;
            if(i==0)
                Write("0",1,Console_Output);
            if(i==1)
                Write("1",1,Console_Output);
            if(i==2)
                Write("2",1,Console_Output);
            if(i==3)
                Write("3",1,Console_Output);
            Write("Client:",7,Console_Output);
            tmp = ConsoleReadLine(buffer,MAX_IB);
            count_w = Write(buffer,tmp,sock[i]);
            if(count_w==0)
                break;
            Write("Server: " ,8, Console_Output);
            Write(buffer, Read(buffer,MAX_IB,sock[i]), Console_Output);
            Write("\n" ,1, Console_Output);
        }
        Close(sock[i]);
    }
    // for (i = 0; i < 4; i++) {
    //     Close(sock[i]);
    // }
    Halt();
}
