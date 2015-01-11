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
#include <asm/byteorder.h>

#include "rox.h"
#include "hash.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BAI Wei baiwei0427@gmail.com");
MODULE_VERSION("1.1");
MODULE_DESCRIPTION("Driver module in end hosts for RoX");
MODULE_SUPPORTED_DEVICE(DEVICE_NAME)

//RuleTable
static struct RuleTable rt;
/// lock for mutli process safety
//spinlock_t inLock;
//spinlock_t outLock;
//spinlock_t globalLock;

//read/write lock
static rwlock_t my_rwlock;

//Load module into kernel
int init_module(void);
//Unload module from kernel
void cleanup_module(void);

//Open virtual characer device "/dev/rox"
static int device_open(struct inode *, struct file *);
//Release virtual characer device "/dev/rox"
static int device_release(struct inode *, struct file *);
//user space-kernel space communication
//static int device_ioctl(struct inode *, struct file *, unsigned int, unsigned long) ;
static int device_ioctl(struct file *, unsigned int, unsigned long) ; //For 2.6.38.3 kernel
//Print rule delivered from user space
static void print_rule(struct Rule*);

//Outgoing packets POSTROUTING
static struct nf_hook_ops nfho_outgoing;
//Incoming packets PREROUTING
static struct nf_hook_ops nfho_incoming;
//Search for action based on a packet and direction (incoming 0 and outgoging 1) 
static struct Action Search(struct sk_buff *,unsigned int);


static struct Action Search(struct sk_buff *skb,unsigned int direction)
{
        struct Action a;
        struct Rule r;
        struct iphdr *ip_header;   //ip header struct
        struct tcphdr *tcp_header; //tcp header struct
        struct udphdr *udp_header; //udp header struct

        //Initialize a null Action
        Init_Action(&a);
        //Initialize a null Rule
        Init_Rule(&r);
        //Initialize direction of Rule (incoming 1 or outgoing 2)
        r.direction=direction;
        ip_header=(struct iphdr *)skb_network_header(skb);
		
        //The packet is not ip packet (e.g. ARP or others)
		if (unlikely(ip_header==NULL))
		{
			return a;
		}
        //Get source and destination ip address 
        r.src_ip=(unsigned int)ip_header->saddr;
        r.dst_ip=(unsigned int)ip_header->daddr;

		//This packet is an incoming packet, we don't need more information, dst ip address is enough
		if(direction==1)
		{
			r.src_ip=0; //we don't need to consider souce ip for incoming packets
			return Search_Table(&rt,&r);
        }
        //Get protocol (TCP,UDP,etc)
        r.protocol=(unsigned int)ip_header->protocol;

        //If protocols are TCP/UDP, we need to get source and destination ports
        if(ip_header->protocol==IPPROTO_TCP) 
		{         
			tcp_header = (struct tcphdr *)((__u32 *)ip_header+ ip_header->ihl);
			r.src_port=htons((unsigned short int) tcp_header->source);
			r.dst_port=htons((unsigned short int) tcp_header->dest);
        } 
		else if(ip_header->protocol==IPPROTO_UDP) 
		{  
			udp_header = (struct udphdr *)((__u32 *)ip_header+ ip_header->ihl);
			r.src_port=htons((unsigned short int) udp_header->source);
			r.dst_port=htons((unsigned short int) udp_header->dest);
        }
        
        //print_rule(&r);
        if(r.src_port>=10000) 
		{
			r.src_port=0;
        } 
		else if(r.dst_port>=10000) 
		{
			r.dst_port=0;
        }
        //Search matching action in RuleTable rt
        return Search_Table(&rt,&r);
}


//Print packet information
void print_packet(struct sk_buff *skb)
{
        struct timeval tv;         //timeval struct used by do_gettimeofday
        unsigned long time_value;  //current time value (microsecond)
        struct iphdr *ip_header;   //ip header struct
        struct tcphdr *tcp_header; //tcp header struct
        struct udphdr *udp_header; //udp header struct
        char src_ip[16];           //source ip address (string)
        char dst_ip[16];           //destination ip address (string)
        unsigned int src_port;     //source tcp/udp port
        unsigned int dst_port;     //destination tcp/udp port

        ip_header=(struct iphdr *)skb_network_header(skb);

        //The packet is not ip packet (e.g. ARP or others)
        if (!ip_header)
        {
                return;
        }

        //Get current time
        do_gettimeofday(&tv);
        time_value=tv.tv_sec*1000000+tv.tv_usec;

        //Get IP addresses from IP header
        snprintf(src_ip, 16, "%pI4", &ip_header->saddr);
        snprintf(dst_ip, 16, "%pI4", &ip_header->daddr);

        if(ip_header->protocol==IPPROTO_TCP) { //TCP

                tcp_header = (struct tcphdr *)((__u32 *)ip_header+ ip_header->ihl);
                //Get source and destination TCP port
                src_port=htons((unsigned short int) tcp_header->source);
                dst_port=htons((unsigned short int) tcp_header->dest);
                printk(KERN_INFO "%lu us TCP packet from %s:%u to %s:%u\n",time_value,src_ip,src_port,dst_ip,dst_port);
                return;

        } else if(ip_header->protocol==IPPROTO_UDP) { //UDP

                udp_header = (struct udphdr *)((__u32 *)ip_header+ ip_header->ihl);
                //Get source and destination UDP port
                src_port=htons((unsigned short int) udp_header->source);
                dst_port=htons((unsigned short int) udp_header->dest);
                printk(KERN_INFO "%lu us UDP packet from %s:%u to %s:%u\n",time_value,src_ip,src_port,dst_ip,dst_port);
                return;

        } else if(ip_header->protocol==IPPROTO_ICMP) { //ICMP

                printk(KERN_INFO "%lu us ICMP packet from %s to %s\n",time_value,src_ip,dst_ip);

        } else { //Others

                printk(KERN_INFO "%lu us Other IP packet from %s to %s\n",time_value,src_ip,dst_ip);

        }
        
}

//PREROUTING for incoming packets
unsigned int hook_func_in(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
        struct Action a;
        struct iphdr *ip_header=NULL;
        unsigned long flags;
		unsigned int tmp=0;
		unsigned char* b=NULL;


		ip_header=(struct iphdr *)skb_network_header(skb);
		if(likely(ip_header!=NULL))
		{
			//Direction of incoming is 1
			read_lock_irqsave(&my_rwlock,flags);
			a=Search(skb,1);
			read_unlock_irqrestore(&my_rwlock,flags);
			if(a.mark!=0)
			{
				skb->mark=a.mark;
			}
			if(a.new_ip!=0||a.dscp!=0)
			{
				//Note that this function skb_make_writable may change sk_buff
				if(!skb_make_writable(skb,sizeof(struct iphdr)))
				{
					printk(KERN_INFO "Not writable\n");
					return NF_ACCEPT;
				}
				//Get new IP header
				ip_header=(struct iphdr *)skb_network_header(skb);
				ip_header->daddr=a.new_ip;
				tmp=a.dscp*4;
				b=(unsigned char*)&tmp;
				ip_header->tos+=*b;
				ip_header->check=0;
				ip_header->check=ip_fast_csum(ip_header,ip_header->ihl);       
			}
        }
        return NF_ACCEPT;
}

//POSTROUTING for outgoing packets
unsigned int hook_func_out(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
        struct Action a;
        struct iphdr *ip_header=NULL;
        unsigned long flags;
		unsigned int tmp=0;
		unsigned char* b=NULL;

		ip_header=(struct iphdr *)skb_network_header(skb);
		if(likely(ip_header!=NULL))
		{
			//Direction of outgoing is 2
			read_lock_irqsave(&my_rwlock,flags);
			a=Search(skb,2);
			read_unlock_irqrestore(&my_rwlock,flags);
			if(a.mark!=0)
			{
				skb->mark=a.mark;
			}
			if(a.new_ip!=0||a.dscp!=0)
			{
				//Note that this function skb_make_writable may change sk_buff
				if(!skb_make_writable(skb,sizeof(struct iphdr)))
				{
					printk(KERN_INFO "Not writable\n");
					return NF_ACCEPT;
				}
				//Get new IP header
				ip_header=(struct iphdr *)skb_network_header(skb);
				ip_header->daddr=a.new_ip;
				tmp=a.dscp*4;
				b=(unsigned char*)&tmp;
				ip_header->tos+=*b;
				ip_header->check=0;
				ip_header->check=ip_fast_csum(ip_header,ip_header->ihl);        
			}
        }
        return NF_ACCEPT;
}


//Print information of a Rule
static void print_rule(struct Rule* r)
{
        printk(KERN_INFO "Direction: %u\n",r->direction); 
        printk(KERN_INFO "Souce IP address: %x\n",r->src_ip); 
        printk(KERN_INFO "Destination IP address: %x\n",r->dst_ip); 
        printk(KERN_INFO "Protocol: %u\n",r->protocol); 
        printk(KERN_INFO "Source port address :%u\n", r->src_port);
        printk(KERN_INFO "Destination port address :%u\n", r->dst_port);
        printk(KERN_INFO "New Destination IP address: %x\n", r->a.new_ip);
        printk(KERN_INFO "Packet marking: %u\n", r->a.mark);
        printk(KERN_INFO "DSCP value: %u\n", r->a.dscp);
}

static int device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) 
{
        struct Rule* r;
        unsigned long flags;
        switch (ioctl_num) 
		{
            //Insert a new rule to rules
            case IOCTL_ROX_INSERT_RULE:
                //printk(KERN_INFO "Inset a new rule\n");
                r=(struct Rule*)ioctl_param;
                //Print new rule
                print_rule(r);
                //Insert this new rule to RuleTable
                write_lock_irqsave(&my_rwlock,flags);
                Insert_Table(&rt,r,GFP_ATOMIC);
                write_unlock_irqrestore(&my_rwlock,flags);
                //Print_Table(&rt);
                break;

            //Delete a rule from rules
            case IOCTL_ROX_DELETE_RULE:
                r=(struct Rule*)ioctl_param;
                //Print new rule
                //print_rule(r);
                //Delete thie rule from RuleTable
                write_lock_irqsave(&my_rwlock,flags);
                Delete_Table(&rt,r);
                write_unlock_irqrestore(&my_rwlock,flags);
                //Print_Table(&rt);
                break;

            //Update a rule
            case IOCTL_ROX_UPDATE_RULE:
                r=(struct Rule*)ioctl_param;
                //Print new rule
                //print_rule(r);
                //Update this new rule to RuleTable
                write_lock_irqsave(&my_rwlock,flags);
                Update_Table(&rt,r);
                write_unlock_irqrestore(&my_rwlock,flags);
                //Print_Table(&rt);
                break;
			
			//Print current rule table
			case IOCTL_ROX_PRINT_RULE:
				Print_Table(&rt);
				break;
        }
        //spin_unlock(&globalLock);
        return SUCCESS;
}

static int device_open(struct inode *inode, struct file *file) 
{
        //printk(KERN_INFO "Device %s is opened\n",DEVICE_NAME);
        try_module_get(THIS_MODULE);
        return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) 
{
        //printk(KERN_INFO "Device %s is closed\n",DEVICE_NAME);
        module_put(THIS_MODULE);
        return SUCCESS;
}


struct file_operations ops = {
    .read = NULL,
    .write = NULL,
   // .ioctl = device_ioctl, //For 2.6.32 kernel
    .unlocked_ioctl = device_ioctl, //For 2.6.38 kernel
    .open = device_open,
    .release = device_release,
};

//Called when module loaded using 'insmod'
int init_module()
{

	int ret;

	//Initialize RuleTable
	Init_Table(&rt);
	//Initialize clock
	rwlock_init(&my_rwlock);

	//PREROUTING
	nfho_incoming.hook = hook_func_in;                    
	nfho_incoming.hooknum = NF_INET_PRE_ROUTING;         
	nfho_incoming.pf = PF_INET;                       
	nfho_incoming.priority = NF_IP_PRI_FIRST;            
	nf_register_hook(&nfho_incoming);                    

	//POSTROUTING
	nfho_outgoing.hook = hook_func_out;                    
	nfho_outgoing.hooknum = NF_INET_POST_ROUTING;       
	nfho_outgoing.pf = PF_INET;                      
	nfho_outgoing.priority = NF_IP_PRI_FIRST;           
	nf_register_hook(&nfho_outgoing);                  
        
	//Register device file
	ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &ops);
	if (ret < 0) 
	{
		printk(KERN_INFO "Registering char device failed with %d\n", MAJOR_NUM);    
		return ret;
	}
	printk(KERN_INFO "Start RoX kernel module\n", MAJOR_NUM);
	return SUCCESS;
}

//Called when module unloaded using 'rmmod'
void cleanup_module()
{
        unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
        nf_unregister_hook(&nfho_incoming);
        nf_unregister_hook(&nfho_outgoing);  
        Empty_Table(&rt);
        printk(KERN_INFO "Stop RoX kernel module\n");

}