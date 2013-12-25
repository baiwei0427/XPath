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
#include <linux/hrtimer.h>
#include <linux/ktime.h>

MODULE_LICENSE("GPL");

//microsecond to nanosecond
#define US_TO_NS(x)	(x * 1E3L)
//millisecond to nanosecond
#define MS_TO_NS(x)	(x * 1E6L)

static struct hrtimer hr_timer;
//Old time value
static struct timeval tv_old;
//Delay
static unsigned long delay_in_us = 300L;


inline unsigned long time_of_interval(struct timeval tv_new,struct timeval tv_old)
{
	return (tv_new.tv_sec-tv_old.tv_sec)*1000000+(tv_new.tv_usec-tv_old.tv_usec);
}

enum hrtimer_restart my_hrtimer_callback( struct hrtimer *timer )
{
	struct timeval tv;           //timeval struct used by do_gettimeofday
	unsigned long time_interval; //time interval
	ktime_t interval,now;

	//Get current time
	do_gettimeofday(&tv);
	//Calculate interval	
	time_interval=time_of_interval(tv,tv_old);
	//Reset tv_old
	tv_old=tv;
	//Print timer interval
	printk(KERN_INFO "%lu\n",time_interval);
	//printk(KERN_INFO "Wake up\n");

	interval = ktime_set( 0, US_TO_NS(delay_in_us));

	now = ktime_get();
	hrtimer_forward(timer,now,interval);
	return HRTIMER_RESTART;
}

int init_module(void) 
{
	ktime_t ktime;
	
	printk("HR Timer module installing\n");

	do_gettimeofday(&tv_old);

	ktime = ktime_set( 0, US_TO_NS(delay_in_us) );

	hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
  
	hr_timer.function = &my_hrtimer_callback;

	hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );

	return 0;
}

void cleanup_module( void )
{
	int ret;

	ret = hrtimer_cancel( &hr_timer );
	if (ret) {
		printk("The timer was still in use...\n");
	} else { 
		printk("HR Timer module uninstalling\n");
	}

}