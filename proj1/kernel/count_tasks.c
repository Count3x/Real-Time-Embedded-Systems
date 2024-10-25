#include<linux/kernel.h>
#include<linux/syscalls.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

SYSCALL_DEFINE1(count_rt_tasks, int *, result)
{
   // init the count (int)
   int cur_result=0;
   // init pointer for process and thread
   struct task_struct *p;
   struct task_struct *t;
   p = &init_task;
   t = &init_task;
   
   // call macros to iterate over all tasks (both processes and threads) 
   for_each_process_thread(p,t)
   {
   	// print all real-time priority processes over 0
        if(p->rt_priority>0){
            cur_result++;
         }
   }
   
   // moving data from kernel to userspace, return count result
   copy_to_user(result, &cur_result, sizeof(cur_result));
   
   return 0;
}
  

//
// Group 16
// Hengshuo
// Zhaoze
//
