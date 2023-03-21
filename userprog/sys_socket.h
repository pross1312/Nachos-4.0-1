#ifndef __USERPROG_SYS_SOCKET_H__ 
#define __USERPROG_SYS_SOCKET_H__ 

#include "kernel.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <errno.h>


const int MAX_IP_ADDR_SIZE = 20;



int SYS_SocketTCP();


int SYS_SocketConnect(int socketID, char* ip, int port);

int SYS_SocketSend(int socketID, char* buffer, int len);

int SYS_SocketReceive(int socketID, char* buffer, int len);

int SYS_SocketClose(int socketID);

#endif