#define MAX_NUMBER 10

struct my_pci_dev {
	char name[32];
	int vendor_id;
	int device_id;
	char revision_id;
	char interrupt_line;
	char latency_timer;
	int command;
};

struct my_task_cputime {
	uint64_t stime;
	uint64_t utime;
	unsigned long long sum_exec_runtime;
	int size;
};

struct necessary_struct {
	struct my_task_cputime cputime;
	int size;
	struct my_pci_dev devices[MAX_NUMBER];
};
