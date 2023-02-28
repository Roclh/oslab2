#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/percpu-defs.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>

#include <linux/filter.h>
#include <linux/ioctl.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/sched/cputime.h>

#include <linux/netdevice.h>
#include <linux/device.h>
#include "necessary_struct.h"

#define WR_VALUE _IOW('a','a',struct necessary_struct*)
#define RD_VALUE _IOR('a','b',struct necessary_struct*)


MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Linux module for lab2");
MODULE_VERSION("1.0");


#define DEVICE_NUMBER 10
dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static struct mutex lock;

/* Function prototypes */
static int      __init kmod_init(void);
static void     __exit kmod_exit(void);

/* ioctl functions */
static int      etx_open(struct inode *inode, struct file *file);
static int      etx_release(struct inode *inode, struct file *file);
static ssize_t  etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/* Functions for getting data */
int fill_structs(int vendor_id, int device_id, int n_pid, struct necessary_struct *my_struct);

/* File operation structure */
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .open           = etx_open,
        .read           = etx_read,
        .write          = etx_write,
        .unlocked_ioctl = etx_ioctl,
        .release        = etx_release,
};

static int etx_open(struct inode *inode, struct file *file) {
	mutex_lock(&lock);
        pr_info("kmod-ioctl: Device file opened.\n");
        return 0;
}

static int etx_release(struct inode *inode, struct file *file) {
	mutex_unlock(&lock);
        pr_info("kmod-ioctl: Device file closed.\n");
        return 0;
}

static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
        pr_info("kmod-ioctl: Reading device file.\n");
        return 0;
}

static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
        pr_info("kmod-ioctl: Writing in device file.\n");
        return len;
}

static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	char value[64];
	int res;
	int vendor_id = 0;
	int device_id = 0;
	int n_pid = 0;
struct necessary_struct *my_struct = vmalloc(sizeof(struct necessary_struct));

         switch(cmd) {
              case WR_VALUE:
                       if( copy_from_user(value, ((struct necessary_struct*) arg).args, sizeof(char)*64) ) {
                              printk(KERN_ALERT "Data Write : Err!\n");
                       }
		       sscanf(value, "%d %d %d", &vendor_id, &device_id, &n_pid);
                       printk(KERN_INFO "Read vendor ID = %d device ID = %d PID = %d\n", vendor_id, device_id, n_pid);
                       res = fill_structs(vendor_id, device_id, n_pid, my_struct);
                    if( copy_to_user((struct necessary_struct*) arg, my_struct, sizeof(struct necessary_struct)) ) {
                                	printk(KERN_ALERT "Data Read : Err!\n");
                     }
			vfree(my_struct);
                       break;
                case RD_VALUE:
                	
                        break;
                default:
                        printk(KERN_ALERT "Default\n");
                        break;
        }
        return 0;
}

int fill_structs(int vendor_id, int device_id, int n_pid, struct necessary_struct *my_struct) {
	u16 dval;
	char byte;
	int i = 0;
struct pci_dev *device_value = vmalloc(sizeof(struct pci_dev));

	device_value = NULL;
	struct my_pci_dev *mpd = vmalloc(DEVICE_NUMBER * sizeof(struct my_pci_dev));

	//vendor_id = PCI_ANY_ID;
	//device_id = PCI_ANY_ID;
	while ((device_value = pci_get_device(vendor_id, device_id, device_value)) && i<DEVICE_NUMBER) {
		printk(KERN_INFO "Device %d:\n", i);
		strncpy(mpd->name, pci_name(device_value), 13); 
		pci_read_config_word(device_value, PCI_VENDOR_ID, &dval);
		mpd->vendor_id = dval;
		printk(KERN_INFO "Vendor_id %d ", mpd->vendor_id);
		pci_read_config_word(device_value, PCI_DEVICE_ID, &dval);
		mpd->device_id = dval;
		printk(KERN_INFO "Device_id %d\n", mpd->device_id);
		pci_read_config_byte(device_value, PCI_REVISION_ID, &byte);
		mpd->revision_id = byte;
		pci_read_config_byte(device_value, PCI_INTERRUPT_LINE, &byte);
		mpd->interrupt_line = byte;
		pci_read_config_byte(device_value, PCI_LATENCY_TIMER, &byte);
		mpd->latency_timer = byte;
		pci_read_config_word(device_value, PCI_COMMAND, &dval);
		mpd->command = dval;
		pci_dev_put(device_value);
		my_struct->devices[i] = *mpd;
		printk(KERN_INFO "New device %s, i = %d\n", mpd->name, i);
		i++;
	}
	my_struct->size = i;

	if (i == 0) {
		printk(KERN_INFO "kmod: The pci device with these vendor and device ID didn't found.\n");
	}

        struct my_task_cputime *cputime = vmalloc(sizeof(struct my_task_cputime));
	struct task_struct *ts = get_pid_task(find_get_pid(n_pid), PIDTYPE_PID);
	if (ts == NULL) {
		printk(KERN_INFO "kmod: The process with PID = %d didn't found.\n", n_pid);
		my_struct->cputime.size = 0;
		vfree(mpd);
		vfree(cputime);
		return -1;
	}

	struct task_struct *t = vmalloc(sizeof(struct task_struct));
	struct signal_struct *sig = ts->signal;
	uint64_t stime = 0, utime = 0, stime_a = sig->stime, utime_a = sig->utime;
	unsigned long long sum_exec_runtime = sig->sum_sched_runtime;

	t = ts;
	do {
		task_cputime(ts, &utime_a, &stime_a);
		utime += utime_a;
		stime += stime_a;
//		sum_exec_runtime += task_sched_runtime(t);
	} while_each_thread(ts, t);
	
	printk(KERN_INFO "utime=%lld stime= %lld \nsum_exec_runtime=%lld", utime, stime, sum_exec_runtime);

	my_struct->cputime.size = 1;
	my_struct->cputime.stime = stime;
	my_struct->cputime.utime = utime;
	my_struct->cputime.sum_exec_runtime = sum_exec_runtime;

	vfree(mpd);
	vfree(cputime);
//	vfree(sig);
//	vfree(ts);
//	vfree(t);

	printk(KERN_INFO "kmod: Data structures were filled.\n");

	return 0;
}


static int __init kmod_init(void) {
    	printk(KERN_INFO "kmod: module is loading.\n");
	mutex_init(&lock);
    
            /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
                printk(KERN_ALERT "kmod: cannot allocate major number\n");
                return -1;
        }
        printk(KERN_INFO "kmod: Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&etx_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&etx_cdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
            pr_err("Cannot create the Device 1\n");
            goto r_device;
        }
        printk(KERN_INFO "kmod: Device driver inserted\n");
        
        return 0;
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}

static void __exit kmod_exit(void) {
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "kmod: module unloaded\n");
}

module_init(kmod_init);
module_exit(kmod_exit);
