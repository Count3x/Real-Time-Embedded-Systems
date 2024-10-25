/*
 *
 * CS 251/EE 255: Real-Time Embedded Systems
 * HW Project #2: Task Scheduling and Monitoring
 * Group 16
 * Hengshuo Zhang
 * Zhaoze Sun
 *
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/syscall.h>
#include <linux/types.h>

// Test File for syscall(453), wait_until_next_period
int main()
{
    printf("Setting RT mode...\n");
    if (syscall(450, getpid(), 200, 1200) != 0) {
        perror("set_rtmon");
        exit(1);
    }
    printf("RT mode set!\n");

    int count = 0;
    while (1) {
        // Perform some computation
        printf("Do computation... (loop %d)\n", count);
        sleep(1);
        count++;

        // Wait until the next period
        long ret = syscall(453);
        if (ret == -EINVAL) {
            printf("Error: task not registered as a periodic task.\n");
            exit(1);
        } else if (ret != 0) {
            printf("Error: wait_until_next_period() returned %ld\n", ret);
            exit(1);
        }
    }
    return 0;
}
