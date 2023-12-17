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
#define MY_PROC_ENTRY "tsulabs"
#define PROC_FULL_PATH "/proc/tsulabs"

struct proc_dir_entry *proc;
int len;
char *msg = NULL;


static ssize_t my_proc_write(struct file *filp, const char __user * buffer, size_t count, loff_t *pos)
{
    int i;
    char *data = pde_data(file_inode(filp));

    if (count > DATA_SIZE) {
        return -EFAULT;
    }

    printk(KERN_INFO "Printing the data passed. Count is %lu", (size_t) count);
    for (i=0; i < count; i++) {
        printk(KERN_INFO "Index: %d . Character: %c Ascii: %d", i, buffer[i], buffer[i]);
    }

    printk(KERN_INFO "Writing to proc");
    if (copy_from_user(data, buffer, count)) {
        return -EFAULT;
    }

    data[count-1] = '\0';

    printk(KERN_INFO "msg has been set to %s", msg);
    printk(KERN_INFO "Message is: ");
    for (i=0; i < count; i++) {
        printk(KERN_INFO "\n Index: %d . Character: %c", i, msg[i]);
    }

    *pos = (int) count;
    len = count-1;

    return count;
}



ssize_t my_proc_read(struct file *filp,char *buf, size_t count, loff_t *offp )
{
    int err;
    char *data = pde_data(file_inode(filp));

    if ((int) (*offp) > len) {
        return 0;
    }

    printk(KERN_INFO "Reading the proc entry, len of the file is %d", len);
    if(!(data)) {
        printk(KERN_INFO "NULL DATA");
        return 0;
    }

    if (count == 0) {
        printk(KERN_INFO "Read of size zero, doing nothing.");
        return count;
    } else {
        printk(KERN_INFO "Read of size %d", (int) count);
    }

    count = len + 1; // +1 to read the \0
    err = copy_to_user(buf, data, count); // +1 for \0
    printk(KERN_INFO "Read data : %s", buf);
    *offp = count;

    if (err) {
        printk(KERN_INFO "Error in copying data.");
    } else {
        printk(KERN_INFO "Successfully copied data.");
    }

    return count;
}




#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_fops = {
    .proc_read = my_proc_read,
    .proc_write = my_proc_write,
};
#else
struct file_operations proc_fops = {
    .read = my_proc_read,
    .write = my_proc_write,
};
#endif





int create_new_proc_entry(void) {

	unsigned long swap_size = (get_nr_swap_pages() * PAGE_SIZE) / (1024 * 1024);

    pr_info("Swap size: %lu MB\n", swap_size);

    int i;
    char DATA[DATA_SIZE];
    snprintf(DATA, sizeof(DATA), "Swap size: %lu MB\n", swap_size);
    len = strlen(DATA);
    msg = kmalloc((size_t) DATA_SIZE, GFP_KERNEL); // +1 for \0


    if (msg != NULL) {
        printk(KERN_INFO "Allocated memory for msg");
    } else {
        return -1;
    }

    strncpy(msg, DATA, len+1);
    for (i=0; i < len +1 ; i++) {
        printk(KERN_INFO "%c", msg[i]);
        if (msg[i] == '\0') {
            printk(KERN_INFO "YES");
        }
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
