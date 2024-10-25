#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/syscall.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>

extern sys_call_ptr_t sys_call_table[];


asmlinkage long(*orig_sys_count_rt_tasks)(int*);
SYSCALL_DEFINE1(mod_count_tasks, int*, result)
{
	int cur_result=0;
	   // init pointer for process and thread
	   struct task_struct *p;
	   struct task_struct *t;
	   p = &init_task;
	   t = &init_task;
	   // call macros to iterate over all tasks (both processes and threads) 
	   for_each_process_thread(p,t)
	   {
	   	// print all real-time priority processes over 50
		if(p->rt_priority>50){
		    cur_result++;
		 }
	   }

   	// return result from kernel to userspace
   	copy_to_user(result, &cur_result, sizeof(cur_result));
   	return 0;
 }






static int mod_count_tasks_init (void){
	   
	   //syscall_table = (unsigned long *)kallsyms_lookup_name("sys_count_rt_tasks");
	   orig_sys_count_rt_tasks=(void*)sys_call_table[__NR_count_rt_tasks];
	   sys_call_table[__NR_count_rt_tasks]=(void*)__x64_sys_mod_count_tasks;
	   return 0;
	
}

static void mod_count_tasks_exit (void){
	
	//syscall_table=(unsigned long *)kallsyms_lookup_name("sys_count_rt_tasks");
	sys_call_table[__NR_count_rt_tasks] = (void*)orig_sys_count_rt_tasks;
}


module_init(mod_count_tasks_init);
module_exit(mod_count_tasks_exit);

MODULE_LICENSE("GPL");


//
// Group 16
// Hengshuo
// Zhaoze
//

