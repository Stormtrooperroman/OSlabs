#include<linux/kernel.h>
#include <linux/module.h> /* Needed by all modules */
#include <linux/printk.h> /* Needed for pr_info() */
#include<linux/proc_fs.h>
#include<linux/uaccess.h>
#include <linux/version.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stormtrooperroman");


static int __init lkm_init(void) {
 printk(KERN_INFO "Welcome to the Tomsk State University\n");
 return 0;
}
static void __exit lkm_exit(void) {
 printk(KERN_INFO "Tomsk State University forever!\n");
}
module_init(lkm_init);
module_exit(lkm_exit);
