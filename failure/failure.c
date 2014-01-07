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
#include <linux/hrtimer.h>
#include <linux/ktime.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BAI Wei baiwei0427@gmail.com");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("Driver module to simulate failure handling of RoX");

//microsecond to nanosecond
#define US_TO_NS(x)	(x * 1E3L)
//millisecond to nanosecond
#define MS_TO_NS(x)	(x * 1E6L)
//Delay
static unsigned long delay_in_us = 1000L;

static unsigned long sum=0;

static unsigned int count=0;

//Failure flag
static int failure=1;

///High resolution timer
static struct hrtimer hr_timer;

//Outgoing packets hook at POSTROUTING
static struct nf_hook_ops nfho_outgoing;
//Incoming packets hook at PREROUTING
static struct nf_hook_ops nfho_incoming;


static enum hrtimer_restart my_hrtimer_callback( struct hrtimer *timer )
{
	//Cancel failure
	failure=0;
	return HRTIMER_NORESTART;
}

static unsigned int hook_func_out(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	struct iphdr *ip_header;   //ip header struct
	struct tcphdr *tcp_header; //tcp header struct
	
	//We don't listen on eth0
    if(strcmp(out->name,"eth0")!=0)
    {
		ip_header=(struct iphdr *)skb_network_header(skb);

		//The packet is not ip packet (e.g. ARP or others)
		if (!ip_header)
		{
			return NF_ACCEPT;
		}
	
		if(ip_header->protocol==IPPROTO_TCP) //TCP packets
		{ 
			tcp_header = (struct tcphdr *)((__u32 *)ip_header+ ip_header->ihl);
			unsigned int src_port=htons((unsigned short int) tcp_header->source);
			//When failure is happening, we must drop all packets of iperf
			if(src_port==5001&&failure==1) 
			{
				printk(KERN_INFO "Drop an outgoing packet whose length is %d\n",skb->len);
				return NF_DROP;
			}
			//When failure has ended, we do DNAT
			else if(src_port==5001&&failure==0)
			{
				//Note that this function skb_make_writable may change sk_buff
                if(!skb_make_writable(skb,sizeof(*ip_header)))
                {
					printk(KERN_INFO "Not writable\n");
					return NF_ACCEPT;
                }
				//Get new IP header
                ip_header=(struct iphdr *)skb_network_header(skb);
                
				//new IP destination is 192.168.22.153
				unsigned int ip=0x9916A8C0;
				ip_header->daddr=ip;
				ip_header->check=0;
                ip_header->check=ip_fast_csum(ip_header,ip_header->ihl);  
				return NF_ACCEPT;
			}
		}
	}
	return NF_ACCEPT;
}


static unsigned int hook_func_in(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	struct iphdr *ip_header;   //ip header struct
	struct tcphdr *tcp_header; //tcp header struct

	//We don't listen on eth0
    if(strcmp(in->name,"eth0")!=0)
    {
		ip_header=(struct iphdr *)skb_network_header(skb);

		//The packet is not ip packet (e.g. ARP or others)
		if (!ip_header)
		{
			return NF_ACCEPT;
		}
	
		if(ip_header->protocol==IPPROTO_TCP) //TCP packets
		{ 
			tcp_header = (struct tcphdr *)((__u32 *)ip_header+ ip_header->ihl);
			unsigned int dst_port=htons((unsigned short int) tcp_header->dest);
			//We only deal with iperf traffic tcp source port=5001 for incoming packets 
			if(dst_port==5001&&failure==1) 
			{
				//return NF_ACCEPT;
				printk(KERN_INFO "Drop an incoming packet whose length is %d\n",skb->len);
				return NF_DROP;
			}
		}
	}
	return NF_ACCEPT;
}


int init_module(void) 
{ 

	//PREROUTING
    nfho_incoming.hook = hook_func_in;                    //function to call when conditions below met
    nfho_incoming.hooknum = NF_INET_PRE_ROUTING;          //called in pre_routing
    nfho_incoming.pf = PF_INET;                           //IPV4 packets
    nfho_incoming.priority = NF_IP_PRI_FIRST;             //set to highest priority over all other hook functions
    nf_register_hook(&nfho_incoming);                     //register hook

	//POSTROUTING
    nfho_outgoing.hook = hook_func_out;                   //function to call when conditions below met
    nfho_outgoing.hooknum = NF_INET_POST_ROUTING;         //called in post_routing
    nfho_outgoing.pf = PF_INET;                           //IPV4 packets
    nfho_outgoing.priority = NF_IP_PRI_FIRST;             //set to highest priority over all other hook functions
    nf_register_hook(&nfho_outgoing);                     //register hook

	ktime_t ktime;
	ktime = ktime_set( 0, US_TO_NS(delay_in_us) );
	hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	hr_timer.function = &my_hrtimer_callback;
	hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );

	//Initialize with failure 
	failure=1;

	printk(KERN_INFO "Failure module is installed\n");
	return 0;
}

void cleanup_module(void)
{
	int ret;

	ret = hrtimer_cancel( &hr_timer );
	if (ret) {
		printk(KERN_INFO "The timer was still in use...\n");
	} 

	nf_unregister_hook(&nfho_outgoing);
	nf_unregister_hook(&nfho_incoming);
		
	printk(KERN_INFO "Failure module is uninstalled\n");
}
