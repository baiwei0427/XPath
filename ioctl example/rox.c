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

#include "rox.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BAI Wei wbaiab@ust.hk");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("Driver module in end hosts for RoX");
MODULE_SUPPORTED_DEVICE(DEVICE_NAME)

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static int device_ioctl(struct inode *, struct file *, unsigned int, unsigned long) ;
static void print_rule(struct Rule*);

static void print_rule(struct Rule* r)
{
	printk(KERN_INFO "Device name: %s\n",r->dev);
	printk(KERN_INFO "Souce IP address: %u\n",r->src_ip); 
	printk(KERN_INFO "Destination IP address: %u\n",r->dst_ip); 
	printk(KERN_INFO "Protocol: %u\n",r->protocol); 
	printk(KERN_INFO "Source port address :%u\n", r->src_port);
	printk(KERN_INFO "Destination port address :%u\n", r->dst_port);
	printk(KERN_INFO "New Destination IP address: %u\n", r->new_ip);
	printk(KERN_INFO "Packet marking: %d\n", r->mark);

}

static int device_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) 
{
	printk(KERN_INFO "Come into device_ioctl with ioctl_num %lu\n",ioctl_num);
	struct Rule* r;
	switch (ioctl_num) {
	//Insert a new rule to rules
	    case IOCTL_ROX_INSERT_RULE:
		printk(KERN_INFO "Inset a new rule\n");
		r=(struct Rule*)ioctl_param;
		print_rule(r);
		//break;
	}
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
    .read = NULL,
    .write = NULL,
    .ioctl = device_ioctl,
    //.unlocked_ioctl = device_ioctl,
    .open = device_open,
    .release = device_release,
};

//Called when module loaded using 'insmod'
int init_module()
{
	int ret;
	//Register device file
	ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &ops);
	if (ret < 0) {
		printk(KERN_INFO "Registering char device failed with %d\n", MAJOR_NUM);
		
		return ret;
	}
	printk(KERN_INFO "Registering char device successfully with %d\n", MAJOR_NUM);
	printk(KERN_INFO "%lu\n",IOCTL_ROX_INSERT_RULE);

	return SUCCESS;
}

//Called when module unloaded using 'rmmod'
void cleanup_module()
{
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	printk(KERN_INFO "Unregistering char device\n");

}