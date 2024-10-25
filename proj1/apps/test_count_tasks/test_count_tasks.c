#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#define __NR_count_tasks 449

int main(int argc, char *argv[])
{
    int result;
    	// make syscall in userspace, Store the kernel return value in userspace and perform the subsequent operations
	int ret = syscall(449, &result);
	if (ret == 0) {
		// success and return the result
		printf("Number of real-time tasks: %d\n", result);
	} else {
		// false and return the error code
		printf("System call failed with error code: %d\n", ret);
	}
	return 0;

    
 }
 
//
// Group 16
// Hengshuo
// Zhaoze
//
