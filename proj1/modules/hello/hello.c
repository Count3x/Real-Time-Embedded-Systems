#include <linux/module.h>  /* Needed by all modules */
#include <linux/init.h>
#include <linux/kernel.h>  /* Needed for KERN_ALERT */

static int hello_init(void){
	// print kernel info in kernel space (register the LKM)
	printk(KERN_ALERT "\tHello world! team16 in kernel space\n");
	return 0;
}

static void hello_exit(void){
	// print kernel info in kernel space (deregister the LKM)
	printk(KERN_ALERT "\tHello world! team16 out kernel space\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");


//
// Group 16
// Hengshuo
// Zhaoze
//
