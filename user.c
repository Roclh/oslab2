#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include "necessary_struct.h"

#define WR_VALUE _IOW('a','a',struct necessary_struct*)
#define RD_VALUE _IOR('a','b',struct necessary_struct*)

int main(int argc, char *argv[]) {
	int32_t f_arg = 0;
	int32_t s_arg = 0;
	int32_t t_arg = 0;

	if (argc == 2) {
		f_arg = -1;
		s_arg = -1;
		t_arg = atoi(argv[1]);	
	} else
	if (argc != 4) {
		printf("Wrong number of args. Write vendor_id, device_id and PID of the task.\n");
		return 0;
	} else {
		f_arg = atoi(argv[1]);
		s_arg = atoi(argv[2]);
		t_arg = atoi(argv[3]);
	}

	if (t_arg <= 0) {
		printf("The PID number must be positive.\n");
		return 0;
	}

	char value[64];
	sprintf(value, "%d %d %d", f_arg, s_arg, t_arg);
	
	struct necessary_struct ns;
	ns->args = value;
	
	int fd;
        printf("Driver is opening.\n");
        fd = open("/dev/etx_device", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }
 
 
        printf("Writing values to Driver\n");
        ioctl(fd, WR_VALUE, (struct necessary_struct *) &ns);
        
	if (ns.size == 0) {
		printf("PCI Device with vendor_id = %d and device_id = %d don't found\n", f_arg, s_arg);
	} else {
        	printf("PCI_DEVICE:\n");
		int i;
		printf("Size %d\n", ns.size);
		for (i = 0; i < 10; i++) {
			printf("\nDevice #%d:\n", i+1);
        		printf("	Name %s\n", ns.devices[i].name);
			printf("	Vendor ID = %d, Device ID = %d\n", ns.devices[i].vendor_id, ns.devices[i].device_id);
			printf("	Revision ID = %d\n", ns.devices[i].revision_id);
			printf("	Interrupt Line = %d\n", ns.devices[i].interrupt_line);
			printf("	Latency Timer = %d\n", ns.devices[i].latency_timer);
			printf("	Command = %d\n", ns.devices[i].command);
		}
	}

	if (ns.cputime.size == 0) {
		printf("The process with PID = %d don't exist.\n", t_arg);
	} else {
		printf("\nTASK_CPUTIME:\n");
		printf("stime = %ld\n", ns.cputime.stime);
		printf("utime = %ld\n", ns.cputime.utime);
		printf("sum_exec_runtime = %lld\n", ns.cputime.sum_exec_runtime);
	}

        printf("Driver is closed.\n");
        close(fd);
	return 0;
}
