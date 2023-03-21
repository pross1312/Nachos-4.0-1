#include "sys_socket.h"
#include "sys_file.h"

extern pair<OpenFile*, int> FileDescriptor[MAX_OPEN_FILES];

// namespace {
//     static int OpenSocketID[20];
// }

int SYS_SocketTCP() {
    int sock;
    int index = findEmptySlot();
    // find position for to store
    // no position
    if (index == -1)
        return -1;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return -1;
    DEBUG(dbgSys, "Create successfully socket at " << index << ", socket: " << sock << ".");
#ifdef FILESYS_STUB
    FileDescriptor[index].first = new OpenFile(sock);
    FileDescriptor[index].second = SOCKET;
#else
    cerr << "Error: FILESYS_STUB is not defined" << endl;
#endif
    return index;
}

int SYS_SocketConnect(int socketID, char* ip, int port) {
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        DEBUG(dbgSys, "Can't parse ip address.");
        return -1;
    }
#ifdef FILESYS_STUB
    if (connect(FileDescriptor[socketID].first->getFileDescriptor(), (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        DEBUG(dbgSys, "Can't connect to server.");
        return -1;
    }
#else
    cerr << "Error: FILESYS_STUB is not defined" << endl;
    return -1;
#endif

    DEBUG(dbgSys, "Connect successfully socket at id: " << socketID);
    return 0;
}


// int SYS_SocketTCP() {
//     int sock;
//     int index = -1;
//     // find position for to store
//     for (int i = 0; i < 20; i++)
//         if (OpenSocketID[i] == 0) {
//             index = i;
//             break;
//         }
//     // no position
//     if (index == -1)
//         return -1;
//     sock = socket(AF_INET, SOCK_STREAM, 0);
//     if (sock < 0)
//         return -1;
//     DEBUG(dbgSys, "Create successfully socket at " << index << ", socket: " << sock << ".");
//     OpenSocketID[index] = sock;
//     return index;
// }

// int SYS_SocketConnect(int socketID, char* ip, int port) {
//     struct sockaddr_in server_addr;
//     memset(&server_addr, 0, sizeof(sockaddr_in));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(port);
//     if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
//         DEBUG(dbgSys, "Can't parse ip address.");
//         return -1;
//     }
//     if (connect(OpenSocketID[socketID], (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
//         DEBUG(dbgSys, "Can't connect to server.");
//         return -1;
//     }
//     DEBUG(dbgSys, "Connect successfully.");
//     return 0;
// }

// int SYS_SocketSend(int socketID, char* buffer, int len) {
//     DEBUG(dbgSys, "Send: " << buffer << " through socket fd: " << OpenSocketID[socketID]);
//     int n = write(OpenSocketID[socketID], buffer, len);
//     if (n < 0) {
//         return -1;
//     }
//     else if (n == 0) {
//         return 0;
//     }
//     DEBUG(dbgSys, n << " bytes sent.");
//     return n;
// }


// int SYS_SocketReceive(int socketID, char* buffer, int len) {
//     DEBUG(dbgSys, "Read message from socket fd: " << OpenSocketID[socketID]);
//     int n = read(OpenSocketID[socketID], buffer, len);
//     if (n < 0)
//         return -1;
//     else if (n == 0)
//         return 0;

//     return n;
// }

// int SYS_SocketClose(int socketID) {
//     DEBUG(dbgSys, "Close socket fd: " << OpenSocketID[socketID]);
//     int result = close(OpenSocketID[socketID]);
//     OpenSocketID[socketID] = 0;
//     return result;
// }