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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/timekeeping.h>


struct periodic_task {
    struct hrtimer timer;
    ktime_t period;
};


enum hrtimer_restart periodic_task_callback(struct hrtimer *timer)
{
    // Get the task struct from the hrtimer pointer using the <container_of> macro.
    struct task_struct *task = container_of(timer, struct task_struct, timer);

    // Update the task's state and schedule it to run again
    set_current_state(TASK_RUNNING);
    wake_up_process(task);

    // Calculate the expiration time of the next period and update the hrtimer
    ktime_t period = ktime_set(task->T, 0);
    ktime_t expiration_time = hrtimer_get_expires(timer);
    hrtimer_forward(timer, expiration_time, period);

    return HRTIMER_RESTART;
}


SYSCALL_DEFINE0(wait_until_next_period)
{
    struct task_struct *task = current;
    

    // Check if the task has been registered as a periodic task
    if (!task->C || !task->T) {
        return -EINVAL;
    }

    // Calculate the expiration time of the next period
    ktime_t now = ktime_get();
    ktime_t period = ktime_set(task->T, 0);
    ktime_t expiration_time = ktime_add(now, period);

    // Check if the current time is already past the expiration time
    if (ktime_compare(now, expiration_time) > 0) {
        return 0;
    }

    // Set up a new hrtimer
    struct hrtimer *timer = &task->timer;
    hrtimer_init(timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS);
    timer->function = &periodic_task_callback;
    hrtimer_forward(timer, expiration_time, period);

    // Set the task state to interruptible and schedule the task
    set_current_state(TASK_RUNNING);
    schedule();


    return 0;
}
