#include "../userprog/syscall.h"

int main() {
    int a = 0;
    int b = 0;
    char* argv1[] = {"Write", "Hello from file 1"};
    char* argv2[] = {"Write", "Hello from file 2"};

    CreateSemaphore("console_out", 1); 
    a = ExecV(2, argv1);
    b = ExecV(2, argv2);
    
    Join(a);
    Join(b);
     
    Exit(0);
}   
