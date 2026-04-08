#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    printf("Starting synchronized yaw control...\n");

    int pid = fork();

    if (pid < 0) {
        printf("Fork failed.\n");
        exit(1);
    }

    if (pid == 0) {
        // Child: Wind Vane Process (Producer)
        char* commands[] = {"45 deg E", "60 deg E", "90 deg S"};
        
        for (int i = 0; i < 3; i++) {
            pause(20); // simulate delay waiting for wind shift
            printf("[VANE] Wind shift: %s. Enqueuing cmd...\n", commands[i]);
            send_yaw(commands[i]);
        }
        
        exit(0);
    } else {
        // Parent: Yaw Motor Process (Consumer)
        char dequeued_cmd[32];
        
        for (int i = 0; i < 3; i++) {
            read_yaw(dequeued_cmd);
            printf("[MOTOR] Dequeued cmd: %s. Actuating...\n", dequeued_cmd);
            pause(30); // simulate the time it takes to actuate the motor
            printf("[MOTOR] Yaw alignment complete.\n");
        }
        
        wait(0);
        exit(0);
    }
}
