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
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/sched/signal.h>
#include <linux/proc_fs.h>
#include <linux/mmzone.h>
#include <linux/vmstat.h>


#define DEVICE_NAME "vm_areas"

// Function called when user writes to the device file
static ssize_t vm_areas_write(struct file *filp, const char __user *ubuf,
                              size_t count, loff_t *f_pos)
{
    char kbuf[20];
    struct task_struct *task;
    struct vm_area_struct *vma;
    int pid, locked;
    unsigned long start, end, size;
    unsigned long resident_pages;

    // Copy input string from user space to kernel space 
    if (copy_from_user(kbuf, ubuf, min(count, sizeof(kbuf)-1)) != 0) {
        printk(KERN_ERR "%s: copy_from_user failed\n", DEVICE_NAME);
        return -EFAULT;
    }

    // Parse PID from input string
    kbuf[sizeof(kbuf)-1] = '\0';
    if (kstrtoint(kbuf, 0, &pid) != 0) {
        printk(KERN_ERR "%s: invalid input '%s'\n", DEVICE_NAME, kbuf);
        return -EINVAL;
    }

    // Get task_struct for the given PID
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (task == NULL) {
        printk(KERN_ERR "%s: process with PID %d not found\n", DEVICE_NAME, pid);
        return -EINVAL;
    }

    // Traverse list of memory areas for the process
    printk(KERN_INFO "%s: Memory-mapped areas of process %d\n", DEVICE_NAME, pid);
    vma = task->mm->mmap;
    while (vma != NULL) {
        locked = (vma->vm_flags & VM_LOCKED) ? 1 : 0;
        start = vma->vm_start;
        end = vma->vm_end;
        size = end - start;
        resident_pages = (get_mm_counter(task->mm, MM_FILEPAGES) +
                          get_mm_counter(task->mm, MM_ANONPAGES)) / PAGE_SIZE;
        printk(KERN_INFO "%lx - %lx: %lu bytes, %lu pages in physical memory%s\n", start, end, size, resident_pages,
               (locked ? " [L]" : ""));
        vma = vma->vm_next;
    }

    return count;
}


// Define the file operations struct for the device file
static struct file_operations vm_areas_fops = {
    .write = vm_areas_write,
};

// Define the miscdevice struct for the device file
static struct miscdevice vm_areas_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &vm_areas_fops,
};

// Module initialization function
static int __init vm_areas_init(void)
{
    int ret;

    // Register the miscdevice
    ret = misc_register(&vm_areas_dev);
    if (ret != 0) {
        printk(KERN_ERR "%s: misc_register failed (error %d)\n", DEVICE_NAME, ret);
        return ret;
    }

    printk(KERN_INFO "%s: loaded\n", DEVICE_NAME);
    return 0;
}

// Module exit function
static void __exit vm_areas_exit(void)
{
    misc_deregister(&vm_areas_dev);
    printk(KERN_INFO "%s: unloaded\n", DEVICE_NAME);
}

module_init(vm_areas_init);
module_exit(vm_areas_exit);
MODULE_LICENSE("GPL");
