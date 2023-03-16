#include "../userprog/syscall.h"



int main() {
    int c = Create("Hello.txt");
    int id = Open("Hello.txt", READ_WRITE);
    int count = Write("Hello world\n", 12, id);
    Close(id);

    return 0;
}   
