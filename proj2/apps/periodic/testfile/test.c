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
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdlib.h>


#define SET_RTMON_SYSCALL 450
#define PRINT_RTMON_SYSCALL 452
#define CANCEL_RTMON_SYSCALL 451


// Test File for syscall(450), set_rtmon
// Test File for syscall(451), cancel_rtmon
// Test File for syscall(452), print_rtmon

int main(int argc, char** argv) {
    pid_t pid;
    unsigned int C_ms, T_ms, sleep_time;
    int res;

    if (argc != 4) {
        printf("Usage: %s <C_ms> <T_ms> <sleep_time>\n", argv[0]);
        return -1;
    }

    pid = getpid();
    C_ms = atoi(argv[1]);
    T_ms = atoi(argv[2]);
    sleep_time = atoi(argv[3]);

    printf("Setting up monitoring for PID %d, C = %u ms, T = %u ms...\n", pid, C_ms, T_ms);

    res = syscall(SET_RTMON_SYSCALL, pid, C_ms, T_ms);
    if (res != 0) {
        printf("Error setting up monitoring: %d\n", res);
        return -1;
    }

    printf("Monitoring set up successfully. Wait for %u seconds to print and cancel usage information.\n", sleep_time);
    sleep(sleep_time);

    res = syscall(PRINT_RTMON_SYSCALL, pid);
    if (res != 0) {
        printf("Error printing usage information: %d\n", res);
        return -1;
    }

    res = syscall(CANCEL_RTMON_SYSCALL, pid);
    if (res != 0) {
        printf("Error canceling monitoring: %d\n", res);
        return -1;
    }

    printf("Monitoring print/cancel successfully.\n");

    return 0;
}


