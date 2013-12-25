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
#include <asm/uaccess.h>

#include "shaper.h"

//1.28M 800Mbps
#define BUCKET 1600000 //800K Bytes
#define RATE 100000000//125000000 //125M Bytes persecond (1Gbps)
#define LOOP_JIFFIES 1

//Tokens in bucket
unsigned long tokens=0;
//Load module into kernel
int init_module(void);
//Unload module from kernel
void cleanup_module(void);
//PacketQueue pointer
static struct PacketQueue *q=NULL;
//Outgoing packets POSTROUTING
static struct nf_hook_ops nfho_outgoing;
/// the timer
struct timer_list timer;
//Old time value
struct timeval tv_old;
//lock
spinlock_t globalLock;

unsigned long interval(struct timeval tv_new,struct timeval tv_old)
{
	return (tv_new.tv_sec-tv_old.tv_sec)*1000000+(tv_new.tv_usec-tv_old.tv_usec);
}

//POSTROUTING for outgoing packets
unsigned int hook_func_out(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	struct timeval tv;           //timeval struct used by do_gettimeofday
	unsigned long time_interval; //time interval
	struct iphdr *ip_header;   //ip header struct
	struct tcphdr *tcp_header; //tcp header struct

	ip_header=(struct iphdr *)skb_network_header(skb);

	//The packet is not ip packet (e.g. ARP or others)
	if (!ip_header)
	{
		return NF_ACCEPT;
	}

	if(ip_header->protocol==IPPROTO_TCP) { //TCP packets
		
		tcp_header = (struct tcphdr *)((__u32 *)ip_header+ ip_header->ihl);
		unsigned int dst_port=htons((unsigned short int) tcp_header->dest);
		//We only deal with iperf traffic tcp dst port=5001 
		if(dst_port==5001) {

			spin_lock(&globalLock);
			/*
			//Get current time
			do_gettimeofday(&tv);
			//Calculate interval	
			time_interval=interval(tv,tv_old);
			//Reset tv_old
			tv_old=tv;
			//Update tokens
			tokens=tokens+time_interval*RATE/1000000;
			//Toekns no larger then bucket size
			if(tokens>=BUCKET)
				tokens=BUCKET;*/
			//There are enough tokens and no packet in queue
			if(tokens>=skb->len&&q->size==0) {
				//Reduce tokens by packet size
				tokens=tokens-skb->len;
				spin_unlock(&globalLock);
				return NF_ACCEPT;
			}
			//Else, we need to enqueue this packet
			int result=Enqueue_PacketQueue(q,skb,okfn);
			spin_unlock(&globalLock);
			
			if(result==1) { //Enqueue successfully
				
				//printk(KERN_INFO "Enqueue a packet\n");
				return NF_STOLEN;

			} else {        //No enough space in queue
				
				printk(KERN_INFO "No enough space in queue\n");
				return NF_DROP;
			
			}
		}
	}
	return NF_ACCEPT;
}

/**
 * handler for timer events.\n
 * send as much as possible packets of a ranges queue and resets its traffic
 * counter if necessary
 * @param data default parameter, unused 
 */
void timer_handler(unsigned long int data) {

	struct timeval tv;           //timeval struct used by do_gettimeofday
	unsigned long time_interval; //time interval
	unsigned int len;

	//Get current time
	do_gettimeofday(&tv);
	//Calculate interval	
	time_interval=interval(tv,tv_old);
	//Reset tv_old
	tv_old=tv;
	//Update tokens	
	tokens=tokens+time_interval*RATE/1000000;

	spin_lock(&globalLock);
	while(1)
	{

		if(q->size>0) { //There are still some packets in queue 
			len=q->packets[q->head].skb->len;
			//printk(KERN_INFO "%u\n",len);
			if(len<tokens) { //There are enough tokens
				//Reduce tokens
				tokens=tokens-len;
				//Deuqueu packets
				Dequeue_PacketQueue(q);
			} else { //There are no enough tokens
				break;
			}
		} else { 
			break;
		}
	}
	//Toekns no larger then bucket size if there are no packets to transmit
	if(tokens>=BUCKET&&q->size==0)
		tokens=BUCKET;
	spin_unlock(&globalLock);
	
	
	timer.expires = jiffies + LOOP_JIFFIES;
    add_timer(&timer);
}


int init_module(void) {

	//Initialize tokens
	tokens=BUCKET;
	//Initialize clock
	spin_lock_init(&globalLock);

	//Init PacketQueue
	q=vmalloc(sizeof(struct PacketQueue));
	Init_PacketQueue(q);

	//Init timer
	do_gettimeofday(&tv_old);

	init_timer(&timer);
    timer.expires = jiffies + LOOP_JIFFIES;
    timer.data = 0;
    timer.function = timer_handler;
    add_timer(&timer);

    //POSTROUTING
	nfho_outgoing.hook = hook_func_out;                   //function to call when conditions below met
	nfho_outgoing.hooknum = NF_INET_POST_ROUTING;         //called in post_routing
	nfho_outgoing.pf = PF_INET;                           //IPV4 packets
	nfho_outgoing.priority = NF_IP_PRI_FIRST;             //set to highest priority over all other hook functions
	nf_register_hook(&nfho_outgoing);                     //register hook*/

	return 0;

}

void cleanup_module(void) {

	del_timer(&timer);
	nf_unregister_hook(&nfho_outgoing);
	Free_PacketQueue(q);

}
