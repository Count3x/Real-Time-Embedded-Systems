/*
 *
 * CS 251/EE 255: Real-Time Embedded Systems
 * HW Project #3: Task Allocation and Virtual Memory Management
 * Group 16
 * Hengshuo Zhang
 * Zhaoze Sun
 *
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/mm.h>

#define DEVICE_NAME "segment_info"

// Function to handle writes to the device
static ssize_t segment_info_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    pid_t pid;
    struct task_struct *task;
    struct mm_struct *mm;
    char kbuf[20];
    int ret;

    // Check that the length of the buffer is not too long
    if (len > 20)
        return -EINVAL;

    // Copy the buffer from user space to kernel space
    ret = copy_from_user(kbuf, buf, len);
    if (ret)
        return -EFAULT;

    // Terminate the string with a null character
    kbuf[len] = '\0';
    
    // Convert the string to a PID
    ret = kstrtoint(kbuf, 10, &pid);
    if (ret)
        return -EINVAL;

    // Find the task_struct corresponding to the given PID
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (!task) {
        printk(KERN_INFO "segment_info: error (process %d not found)\n", pid);
        return -EINVAL;
    }

    // Get the memory descriptor for the task
    mm = task->mm;
    if (!mm) {
        printk(KERN_INFO "segment_info: error (no memory descriptor for process %d)\n", pid);
        return -EINVAL;
    }

    // Print the memory segment addresses for the task
    printk(KERN_INFO "[Memory segment addresses of process %d]\n", pid);
    printk(KERN_INFO "%08lx - %08lx: code segment (%ld bytes)\n", mm->start_code, mm->end_code, mm->end_code - mm->start_code);
    printk(KERN_INFO "%08lx - %08lx: data segment (%ld bytes)\n", mm->start_data, mm->end_data, mm->end_data - mm->start_data);

    return len;
}


// Define the file operations for the device
static const struct file_operations segment_info_fops = {
    .owner = THIS_MODULE,
    .write = segment_info_write,
};

// Define the misc device structure for the device
static struct miscdevice segment_info_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &segment_info_fops,
};

// Initialization function for the module
static int __init segment_info_init(void)
{
    // Register the misc device
    int ret = misc_register(&segment_info_miscdev);
    if (ret) {
        printk(KERN_ALERT "Failed to register misc device: %s\n", DEVICE_NAME);
    }
    return ret;
}

// Cleanup function for the module
static void __exit segment_info_exit(void)
{
    misc_deregister(&segment_info_miscdev);
}

module_init(segment_info_init);
module_exit(segment_info_exit);
MODULE_LICENSE("GPL");
