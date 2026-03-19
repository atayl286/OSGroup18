#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void child()
{
    
}

void parent()
{

}

int
main(int argc, char *argv[])
{
    /*
    int pid = fork();
    if(pid == 0)
        child();
    else
        parent();

    */
    prioritize(30);
  exit(0);
}

