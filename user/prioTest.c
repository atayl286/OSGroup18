#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void child(int prio)
{
    int count = 0;
    while(count < 1000000000)
        count++;

    printf("Finished child with %d priority\n", prio);
}

int
main(int argc, char *argv[])
{
    prioritize(30);
    for(int i = 0; i < 10; i++)
    {
        int pid = priofork(i);

        if(pid == 0)
        {
            child(i);
            exit(0);
        }
        else
        {
            printf("Started child with priorty %d\n", i);
        }
    }

    for(int i = 0; i < 10; i++)
        wait(0);

    exit(0);
}

