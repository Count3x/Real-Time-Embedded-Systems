/*
 *
 * CS 251/EE 255: Real-Time Embedded Systems
 * HW Project #3: Task Allocation and Virtual Memory Management
 * Group 16
 * Hengshuo Zhang
 * Zhaoze Sun
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <memory_size>\n", argv[0]);
        return 1;
    }
    
    uint64_t size = atol(argv[1]);	// Convert the memory size argument to an unsigned 64-bit integer
    char *buf = malloc(size);		// Allocate memory with the given size using malloc()
    // If malloc() returns NULL, print an error message and return with an error code
    if (buf == NULL) {
        printf("Error: Failed to allocate memory\n");
        return 1;
    }

    // Lock the allocated memory pages into RAM using mlock()
    // If mlock() fails, print an error message and return with an error code
    if (mlock(buf, size) != 0) {
        printf("%s: mlockall failed\n", argv[0]);
        return 1;
    }

    // Get the current time using clock_gettime() and the CLOCK_MONOTONIC clock
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    // Start filling the allocated memory with ones, one page (4096 bytes) at a time
    for (int i = 0; i < size; i += 4096) {
        buf[i] = 1;
    }

    // Get the current time again using clock_gettime() and calculate the elapsed time in nanoseconds
    clock_gettime(CLOCK_MONOTONIC, &t2);
    uint64_t elapsed = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec;
    printf("PID %d, %lu ns\n", getpid(), elapsed);

    // Wait indefinitely for a signal
    pause();

    // Unlock the allocated memory pages using munlock()
    munlock(buf, size);
    free(buf);
    return 0;
}

