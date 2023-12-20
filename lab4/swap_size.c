#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/types.h>
#include <linux/version.h>
#include <linux/swap.h>

#define DATA_SIZE 1024
#define MY_PROC_ENTRY "tsulab"
#define PROC_FULL_PATH "/proc/tsulab"

struct proc_dir_entry *proc;
int len, temp;
char *msg = NULL;



ssize_t my_proc_read(struct file *filp,char *buf,size_t count,loff_t *offp ) 
{
    int err;
    char *data = pde_data(file_inode(filp));
    printk(KERN_INFO "Reading the proc entry, len of the file is %d", len);
    if(!(data)){
        printk(KERN_INFO "Null data");
        return 0;
    }
    
    printk(KERN_INFO "%lu", count);
    if(count>temp)
    {
        count=temp;
    }
    temp = temp - count;

    err = copy_to_user(buf,data, len);


    printk(KERN_INFO "Read data : %s", buf);
    *offp = count;

    if (err) {
        printk(KERN_INFO "Error in copying data.");
    } else {
        printk(KERN_INFO "Successfully copied data.");
    }
    if(count==0)
        temp = len;
    return count;
}




#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_fops = {
    .proc_read = my_proc_read
};
#else
struct file_operations proc_fops = {
    .read = my_proc_read
};
#endif


int create_new_proc_entry(void) {

	unsigned long swap_size = (get_nr_swap_pages() * PAGE_SIZE) / (1024 * 1024);

    pr_info("Swap size: %lu MB\n", swap_size);

    int i;
    char DATA[DATA_SIZE];
    snprintf(DATA, sizeof(DATA), "Swap size: %lu MB\n", swap_size);
    len = strlen(DATA);
    temp=len;
    msg = kmalloc((size_t) DATA_SIZE, GFP_KERNEL);


    if (msg != NULL) {
        printk(KERN_INFO "Allocated memory for msg");
    } else {
        return -1;
    }

    strncpy(msg, DATA, len);
    for (i=0; i < len; i++) {
        printk(KERN_INFO "%c", msg[i]);
    }

    proc = proc_create_data(MY_PROC_ENTRY, 0666, NULL, &proc_fops, msg);
    if (proc) {
        return 0;
    }
    return -1;
}


int proc_init (void) {
    if (create_new_proc_entry()) {
        return -1;
    }
    return 0;
}

void proc_cleanup(void) {
    remove_proc_entry(MY_PROC_ENTRY, NULL);
}

module_init(proc_init);
module_exit(proc_cleanup);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stormtrooperroman");
MODULE_DESCRIPTION("Swap Size Module");
