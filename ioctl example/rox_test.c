#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/inet.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <linux/netfilter_ipv4.h>
#include <linux/string.h>
#include <linux/time.h>  
#include <linux/fs.h>
#include <linux/random.h>
#include <linux/errno.h>
#include <linux/timer.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h> /* copy_from/to_user */


MODULE_LICENSE("GPL");
MODULE_AUTHOR("BAI Wei wbaiab@ust.hk");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("Driver module in end hosts for RoX");
MODULE_SUPPORTED_DEVICE(DEVICE_NAME)

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

//Name of character device
#define DEVICE_NAME "rox"
#define SUCCESS 0
#define BUFF_SIZE 1024

static char rx_buffer[BUFF_SIZE]={0};
static char tx_buffer[BUFF_SIZE]={0};
static int Major;
static int size=0;

static ssize_t device_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
	ssize_t bytes;
	if (size < count)
        bytes = size;
    else
        bytes = count;

	printk("user requesting data, our buffer has (%d) \n", size);

	/* Check to see if there is data to transfer */
    if (bytes == 0)
        return 0;

    /* Transfering data to user space */ 
    int retval = copy_to_user(buf,rx_buffer,bytes);

	if (retval) {
        printk("copy_to_user() could not copy %d bytes.\n", retval);
        return -EFAULT;

    } else {
        printk("copy_to_user() succeeded!\n");
        size -= bytes;
        return bytes;
    }
}

static ssize_t device_write(struct file *file, const char *buf, size_t count, loff_t *f_pos)
{
	//zero the input buffer
	memset(tx_buffer,0,BUFF_SIZE);
    memset(rx_buffer,0,BUFF_SIZE);

	printk(KERN_INFO "New message from userspace - count:%d\n",count);
	
    int retval = copy_from_user(tx_buffer,buf,count);

	printk("copy_from_user returned (%d) we read [%s]\n",retval , tx_buffer);
    printk("initialize rx buffer..\n");

	memcpy(rx_buffer,tx_buffer, count);
    printk(KERN_INFO "content of rx buffer [%s]\n", rx_buffer);

	size=count;

	return count;
}

int device_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) 
{
	return SUCCESS;
}

static int device_open(struct inode *inode, struct file *file) 
{
	printk(KERN_INFO "Device %s is opened\n",DEVICE_NAME);
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) 
{
	printk(KERN_INFO "Device %s is closed\n",DEVICE_NAME);
	module_put(THIS_MODULE);
	return SUCCESS;
}


struct file_operations ops = {
    .read = device_read,
    .write = device_write,
    //.ioctl = device_ioctl,
    .unlocked_ioctl = device_ioctl,
    .open = device_open,
    .release = device_release,
};

//Called when module loaded using 'insmod'
int init_module()
{
	//Register device file and get major number
	Major = register_chrdev(0, DEVICE_NAME, &ops);
	if (Major < 0) {
		printk(KERN_INFO "Registering char device failed with %d\n", Major);
		return Major;
	}
	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return SUCCESS;
}

//Called when module unloaded using 'rmmod'
void cleanup_module()
{
	unregister_chrdev(Major, DEVICE_NAME);

}



