#include <fcntl.h> // for open
#include <stdio.h> // for close
#include <unistd.h>
#include <sys/ioctl.h>
#include "ioctl.h"

#define IOCTL_PRINT_HELLO _IO(0,0)
#define IOCTL_GET_TIME_NS _IO(0,1)


int main(){
	int fd;
	unsigned long current_time;
	
	//  open a file with the name "/dev/rtesdev" in read-only mode using the open() system call
	fd = open("/dev/rtesdev", O_RDONLY);
	if(fd < 0){
		perror( "ioctl: ioctl open failed \n");
		return 1;
	}
	 
	// call the ioctl() function with the file descriptor fd and the request number IOCTL_PRINT_HELLO. 
	ioctl(fd, IOCTL_PRINT_HELLO);
	// calling the ioctl() function with the file descriptor fd, the request number IOCTL_GET_TIME_NS, and a pointer to a variable current_time
	ioctl(fd, IOCTL_GET_TIME_NS, &current_time);
	printf( "Current Time in ns: %ld\n", current_time);

	close(fd);
	return 0;

}


//
// Group 16
// Hengshuo
// Zhaoze
//
