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
#include <linux/hrtimer.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/time.h>  
#include <linux/fs.h>
#include <linux/random.h>
#include <linux/errno.h>
#include <linux/timer.h>

#include "queue.h"

MODULE_LICENSE("GPL");

//microsecond to nanosecond
#define US_TO_NS(x)	(x * 1E3L)
//millisecond to nanosecond
#define MS_TO_NS(x)	(x * 1E6L)

unsigned int rate=100; //Mbps

const unsigned int bucket=64000;	//bytes
unsigned int tokens=64000;	//bytes
static struct PacketQueue *queuePtr;
static char *param_dev="eth1\0";
static struct nf_hook_ops nfho_outgoing;
static struct tasklet_struct xmit_timeout;
static struct hrtimer hr_timer;
static ktime_t last_update_time;
static unsigned long delay_in_us = 100L;

static void xmit_tasklet(unsigned long data)
{
	unsigned int skb_len;
	ktime_t now=ktime_get();
	
	tokens=tokens+ktime_us_delta(now,last_update_time)*rate/8;
	last_update_time=now;
	while(1)
	{
		if(queuePtr->len>0)
		{
			skb_len=queuePtr->packets[queuePtr->head].skb->len;
			if(skb_len<=tokens)
			{
				tokens=tokens-skb_len;
				Dequeue_PacketQueue(queuePtr);
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	
	if(tokens>=bucket&&queuePtr->len==0)
		tokens=bucket;	
	//printk(KERN_INFO "Current time is %u\n",(unsigned int)(ktime_to_ns(ktime_get())>>10));	
	//Start time again
	hrtimer_start(&hr_timer, ktime_set( 0, US_TO_NS(delay_in_us)), HRTIMER_MODE_REL);
}

static enum hrtimer_restart my_hrtimer_callback( struct hrtimer *timer )
{
	tasklet_schedule(&xmit_timeout);
	return HRTIMER_NORESTART;
}

//POSTROUTING for outgoing packets
static unsigned int hook_func_out(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	int result=0;
	unsigned long flags;
	
	if(!out)
		return NF_ACCEPT;
        
	if(strcmp(out->name,param_dev)!=0)
		return NF_ACCEPT;
	
	if(ip_hdr(skb)!=NULL)
	{
		spin_lock_irqsave(&(queuePtr->queue_lock),flags);
		result=Enqueue_PacketQueue(queuePtr,skb,okfn);
		spin_unlock_irqrestore(&(queuePtr->queue_lock),flags);
		if(result==0)
		{
			return NF_DROP;
		}
		else
		{
			return NF_STOLEN;
		}
	}
	return NF_ACCEPT;
}

int init_module()
{	
	queuePtr=kmalloc(sizeof(struct PacketQueue),GFP_KERNEL);
	Init_PacketQueue(queuePtr,GFP_KERNEL);
	
	tasklet_init(&xmit_timeout, xmit_tasklet, 0);
	
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function = &my_hrtimer_callback;
	hrtimer_start( &hr_timer, ktime_set( 0, US_TO_NS(delay_in_us) ), HRTIMER_MODE_REL);
	last_update_time=ktime_get();
	
	//NF_POST_ROUTING Hook
	nfho_outgoing.hook = hook_func_out;						
	nfho_outgoing.hooknum =  NF_INET_LOCAL_OUT;//NF_INET_POST_ROUTING;	
	nfho_outgoing.pf = PF_INET;											
	nfho_outgoing.priority = NF_IP_PRI_FIRST;			
	nf_register_hook(&nfho_outgoing);			
	return 0;
}

void cleanup_module()
{
	nf_unregister_hook(&nfho_outgoing);  
	hrtimer_cancel(&hr_timer);
	tasklet_kill(&xmit_timeout);
	Free_PacketQueue(queuePtr);
	vfree(queuePtr);
}