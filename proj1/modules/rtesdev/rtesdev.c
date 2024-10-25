#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init_task.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/ioctl.h>

#define IOCTL_PRINT_HELLO _IO(0,0)
#define IOCTL_GET_TIME_NS _IO(0,1)

typedef s64 ktime_t;




static int rtesdev_open(struct inode *inode, struct file *file){
	// file operator for open misc device
	printk(KERN_ALERT "rtesdev_open: open!\n");
	return 0;
}

static int rtesdev_close(struct inode *inodep, struct file *filp){
	// file operator for close misc device
	printk(KERN_ALERT "rtesdev_close: close!\n");
	return 0;
}

static ssize_t rtesdev_read(struct file *filp, char __user *ubuf, size_t count, loff_t *off){
	// init pointer for process and thread
	struct task_struct *p;
	struct task_struct *t;
	p = NULL;
	p = &init_task;
	t = NULL;
	t = &init_task;
	// info print for identify we are call read operator
	pr_info("Now Reading... ... ...");
	// Print the pid, tid, real-time priority, and name requirements
	printk(KERN_ALERT "tid\tpid\tpr\tname\t");

	// call macros to iterate over all tasks (both processes and threads) 
	for_each_process_thread(p, t){
		// print all real-time priority processes over 0
		if(p->rt_priority > 0){
			printk(KERN_ALERT "%d\t%d\t%dl\t%s\n",t->tgid, p->pid, p->rt_priority, p->comm);
		}
	}
	return 0;
}

// IOCTL stands for I/O control. here
long rtesdev_ioctl(struct file *file,unsigned int cmd, unsigned long arg){
	// init (ktime_t) current time
	ktime_t current_time;

	// in Different IOCTL _IO 
	switch(cmd){
		// IOCTL_PRINT_HELLO for print info in kernel message
		case IOCTL_PRINT_HELLO:
			printk(KERN_ALERT "\tHello world! team16 in kernel space\n");
			break;
		// IOCTL_GET_TIME_NS for print real time in kernel
		case IOCTL_GET_TIME_NS:
			// get kernel time in (ns)
			current_time = ktime_get() * 1000;
			// return result from kernel to userspace
			if(copy_to_user((ktime_t*) arg, &current_time,sizeof(current_time))){}
			//printk(KERN_ALERT "\t%lld\n",current_time);
			break;
		default:
			return -EINVAL;
	}
	return 0;
}


struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = rtesdev_read,
	.open = rtesdev_open,
	.release = rtesdev_close,
	.unlocked_ioctl = rtesdev_ioctl,
};


struct miscdevice rtesdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "rtesdev",
	.fops = &fops,
};




static int rtesdev_init(void){
	int error;

	printk(KERN_INFO "rtesdev(team16): misc_register registered\n");
	error = misc_register(&rtesdev);
	return 0;
}

static void rtesdev_exit(void){
	misc_deregister(&rtesdev);
	printk(KERN_INFO "rtesdev(team16): misc_register deregistered\n");
}


module_init(rtesdev_init);
module_exit(rtesdev_exit);

MODULE_LICENSE("GPL");

//
// Group 16
// Hengshuo
// Zhaoze
//
