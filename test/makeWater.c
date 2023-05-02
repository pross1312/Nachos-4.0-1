#include "../userprog/syscall.h"
#define MAX_ARR 100
int main() {
    int id, id2;
    
    CreateSemaphore("make_O", 0);
    CreateSemaphore("make_H",0);
    id = Exec("copy");
    id2 = Exec("cat");
    Wait("make_H");
    Wait("make_H");
    Wait("make_O");
    Write("Create h20\n", 11, Console_Output);
    return 0;
}
