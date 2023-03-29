#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <algorithm>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(){
    int server_fd, new_socket, valread=1;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char response[BUFFER_SIZE] = {0};
       
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        return(1);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        return(1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        return(1);
    }

    if (listen(server_fd, 4) < 0){
        return(1);
    }

    while(1){

        char buffer[1024] = {0};
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
            return(1);
        }
        while ((valread = read(new_socket, buffer, 1024)) > 0){
            std::cout << "Client: " << buffer << "\n";
            for (int i = 0; buffer[i]; i++){
                buffer[i] = toupper(buffer[i]);
            }
            send(new_socket, buffer, strlen(buffer), 0);
            std::cout << "Server: " << buffer << "\n";
            memset(buffer, 0, sizeof(buffer));
        }
        if (valread == 0){
            std::cout << "Da dong ket noi\n";
        } else{
            std::cout << "doc loi\n";
        }
        close(new_socket);
    }

    return 0;
}
