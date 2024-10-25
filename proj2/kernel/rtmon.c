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


struct rtmon_usage {
    struct  hrtimer timer;	// HRTimer used to track CPU usage
    ktime_t start_time;	// Start time of the current time slice
    ktime_t end_time;		// End time of the current time slice
    ktime_t period;		// Period of the task
    u64     cpu_time;		// Accumulated CPU time
    struct task_struct* task;	// The task being monitored
};

static enum hrtimer_restart rtmon_timer_callback(struct hrtimer* timer) {
    struct rtmon_usage* usage;
    struct task_struct* task;
    ktime_t now, delta;

    // Get the rtmon_usage and task from the timer
    usage = container_of(timer, struct rtmon_usage, timer);
    task = usage->task;
    // Calculate the time elapsed since the last time slice and add it to the CPU time accumulator
    now = ktime_get();
    delta = ktime_sub(now, usage->start_time);
    usage->cpu_time += ktime_to_ns(delta);

    // If the time slice has elapsed, reset the CPU time accumulator and start a new time slice
    if (ktime_after(now, ktime_add(usage->start_time, usage->period))) {
        usage->cpu_time = 0;
        usage->start_time = now;
    }

    // Clear the usage count for the task
    refcount_set(&task->usage, 0);
    // Restart the timer
    hrtimer_forward_now(timer, usage->period);
    return HRTIMER_RESTART;
}




SYSCALL_DEFINE3(set_rtmon, pid_t, pid, unsigned int, C_ms, unsigned int, T_ms) {
    struct task_struct* task, * iter;
    ktime_t response_time, deadline;
    int is_schedulable = 1;
    struct rtmon_usage* usage;

    // Check validate the input parameters
    if (C_ms > 10000 || T_ms > 10000 || C_ms < 1 || T_ms < 1 || C_ms > T_ms)
        return -1;

    // Find the task by PID
    task = find_task_by_pid_ns(pid, &init_pid_ns);
    if (!task)
        return -1;

    // If the task already has C and T values, return an error
    if (task->C || task->T)
        return -1;

    // Set the task's C and T values
    task->C = ktime_set(0, C_ms * NSEC_PER_MSEC);
    task->T = ktime_set(0, T_ms * NSEC_PER_MSEC);

    // Set up an HRTimer to track CPU usage
    usage = kmalloc(sizeof(struct rtmon_usage), GFP_KERNEL);
    if (!usage)
        return -ENOMEM;

    usage->cpu_time = 0;		// Init CPU usage
    usage->start_time = ktime_get();	// Set the current time,  start time for measuring the CPU usage
    usage->period = task->T;		// Set the task's T value, period of the RT monitoring window
    usage->task = task;		// Set the task being monitored
    hrtimer_init(&usage->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);	// Init timer / clock
    usage->timer.function = &rtmon_timer_callback;			// Set the callback function
    hrtimer_start(&usage->timer, usage->period, HRTIMER_MODE_REL);	// Starts the timer
    task->utime = (unsigned long)usage;	// Retrieve the usage data associated with the task

    // Reset the utime of all tasks with non-zero C and T values
    for_each_process(iter) {
        if (iter->C || iter->T) {
            usage = (struct rtmon_usage*)iter->utime;
            if (usage) {
                usage->cpu_time = 0;
                usage->start_time = ktime_get();
                usage->period = iter->T;

                hrtimer_start(&usage->timer, usage->period, HRTIMER_MODE_REL);
            }
        }
    }

    // Perform response time test
    response_time = task->C;
    deadline = task->T;
    while (1) {
        is_schedulable = 1;
        for_each_process(iter) {
            if (iter->__state == TASK_RUNNING && iter->pid != task->pid) {
                if (iter->C && iter->T) {
                    ktime_t w = ktime_add(response_time, iter->C);
                    if (ktime_before(w, iter->T)) {
                        is_schedulable = 0;
                        break;
                    }
                }
            }
        }
        // Check if the response time test has passed or if the deadline has been exceeded
        if (is_schedulable || ktime_after(response_time, deadline)) {
            break;
        }
        // Calculate the new response time for each task based on the maximum 
        response_time = ktime_set(0, 0);
        for_each_process(iter) {
            if (iter->__state == TASK_RUNNING && iter->pid != task->pid) {
                if (iter->C && iter->T) {
                    ktime_t w = ktime_add(response_time, iter->C);
                    response_time = ktime_add(iter->C, ktime_set(0, ktime_divns(ktime_add(w, iter->C), ktime_to_ns(iter->T))));
                }
            }
        }
    }
    // If the response time test failed, reset the task's C and T values and cancel the HRTimer
    if (!is_schedulable) {
        task->C = 0;
        task->T = 0;
        hrtimer_cancel(&usage->timer);
        kfree(usage);
        return -1;
    }

    return 0;
}


SYSCALL_DEFINE1(print_rtmon, pid_t, pid) {
    struct task_struct* task;		// A pointer to a task_struct structure
    struct rtmon_usage* usage;	// A pointer to a rtmon_usage structure
    struct timespec64 current_time;	// The current time
    ktime_t current_ktime;		// The current kernel time
    u64 usage_ns, period_ns, usage_ms;		// CPU usage, period, and usage time in milliseconds
    ktime_t start_time, usage_ktime, current_delta;	// The start time, usage time, and current delta

    // If pid is -1, print usage information for all tasks with non-zero C and T values
    if (pid == -1) {
        for_each_process(task) {
            if (task->C || task->T) {
                usage = (struct rtmon_usage*)task->utime;
                // Get the current time and calculate the usage in milliseconds
                current_ktime = ktime_get();
                current_time = ktime_to_timespec64(current_ktime);

                usage_ns = usage->cpu_time;
                period_ns = ktime_to_ns(usage->period);
                if (usage_ns > period_ns) {
                    usage_ns = period_ns;
                }

                usage_ms = div_u64(usage_ns, NSEC_PER_MSEC);

                // Calculate the usage time based on the current time and the task's start time and CPU time accumulator
                start_time = usage->start_time;
                usage_ktime = ktime_set(0, usage_ns);
                current_delta = ktime_sub(current_ktime, start_time);
                usage_ktime = ktime_add(usage_ktime, current_delta);
                usage_ms = div_u64(ktime_to_ns(usage_ktime), NSEC_PER_MSEC);

                // Print the usage information
                printk(KERN_INFO "print_rtmon: PID %d, C %llu ms, T %llu ms, usage %llu ms\n",
                    task_pid_nr(task), ktime_to_ms(task->C), ktime_to_ms(task->T), usage_ms);
            }
        }
    }
    else { // Print the usage information for a specific task identified by pid
        // If pid is not -1, print usage information for the specified task
        task = find_task_by_pid_ns(pid, &init_pid_ns);
        if (!task)
            return -ESRCH;

        usage = (struct rtmon_usage*)task->utime;
        // Get the current time and calculate the usage in milliseconds
        current_ktime = ktime_get();
        current_time = ktime_to_timespec64(current_ktime);

        usage_ns = usage->cpu_time;
        period_ns = ktime_to_ns(usage->period);
        if (usage_ns > period_ns) {
            usage_ns = period_ns;
        }

        usage_ms = div_u64(usage_ns, NSEC_PER_MSEC);

        // Calculate the usage time based on the current time and the task's start time and CPU time accumulator
        start_time = usage->start_time;
        usage_ktime = ktime_set(0, usage_ns);
        current_delta = ktime_sub(current_ktime, start_time);
        usage_ktime = ktime_add(usage_ktime, current_delta);
        usage_ms = div_u64(ktime_to_ns(usage_ktime), NSEC_PER_MSEC);

        // Print the usage information
        printk(KERN_INFO "print_rtmon: PID %d, C %llu ms, T %llu ms, usage %llu ms\n",
            task_pid_nr(task), ktime_to_ms(task->C), ktime_to_ms(task->T), usage_ms);
    }
    return 0;
}


SYSCALL_DEFINE1(cancel_rtmon, pid_t, pid) {
    struct task_struct* task;
    struct rtmon_usage* usage;

    // Find the task associated with the PID
    task = find_task_by_pid_ns(pid, &init_pid_ns);
    if (!task)
        return -ESRCH;

    // Check if the task has a valid C and T values
    if (!task->C || !task->T)
        return -EINVAL;

    // Cancel the timer and free the usage struct
    usage = (struct rtmon_usage*)task->utime;
    if (usage) {
        hrtimer_cancel(&usage->timer);
        kfree(usage);
        task->C = 0;
        task->T = 0;
        task->utime = 0;
    }

    return 0;
}

