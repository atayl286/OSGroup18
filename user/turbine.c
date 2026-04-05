#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_PRIORITY 10 // Alarm priority
#define CRITICAL_THRESHOLD 100 // Threshold for the alarm trigger (spike in read_sensor is 150)

// Critical processes that must happen immediately:
void emergency_shutdown(int rpm) {
    printf("\n!!! CRITICAL ALARM !!!\n");
    printf("Turbine speed at catastrophic levels!\n");
    printf("Deploying emergency brakes...\n");

    write_log(rpm, LOG_EVENT_SHUTDOWN);

    // Simulating a lot of CPU work:
    int i, j, k = 0;
    for(i = 0; i < 10000; i++) {
        for(j = 0; j < 1000; j++) {
            k += 1; // Dummy work to prevent the compiler from optimizing the loop
        }
    }

    printf("Crisis averted.\n");
    exit(0);
}

int main(void) {
    printf("Starting turbine monitor...\n");

    while(1) {
        int speed = read_sensor();
        
        if (speed < CRITICAL_THRESHOLD) { // Under threshold (normal behaviour)
            write_log(speed, LOG_EVENT_NORMAL);
            printf("Turbine speed normal: %d rpm\n", speed);
            pause(50); // Sleep 50 ticks
        } else { // Over threshold (bad!)
            write_log(speed, LOG_EVENT_SPIKE);
            printf("\n--- SENSOR SPIKE DETECTED: %d rpm ---\n", speed);
            int pid = priofork(MAX_PRIORITY); // Emergency handler
            
            // Failed :(
            if (pid < 0) {
                printf("Emergency fork failed!\n");
                exit(0);
            } 
            
            // High-priority alarm child:
            else if (pid == 0) {
                emergency_shutdown(speed);
            } 
            
            // Standard-priority parent:
            else {
                wait(0); // Wait for emergency handling child to finish
                printf("Monitor resuming normal operations...\n\n");
            }
        }
    }
    
    exit(0);
}
