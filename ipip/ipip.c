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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BAI Wei baiwei0427@gmail.com");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("Driver module in end hosts for IP in IP");

//Outgoing packets POSTROUTING
static struct nf_hook_ops nfho_outgoing;
//Incoming packets PREROUTING
static struct nf_hook_ops nfho_incoming;


//POSTROUTING for outgoing packets
static unsigned int hook_func_out(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	struct iphdr  *old_iph=(struct iphdr *)skb_network_header(skb);
	struct iphdr  *iph;			    /* Our new IP header */
	unsigned int max_headroom;		/* The extra header space needed */

	//We don't listen on eth0
	if(strcmp(out->name,"eth0")!=0)
	{
		if(old_iph!=NULL)
		{
			//We only deal with ICMP packets
			if(old_iph->protocol==IPPROTO_ICMP)
			{	
				max_headroom =sizeof(struct iphdr)+LL_RESERVED_SPACE(out);

				if (skb_headroom(skb) < max_headroom)
				{
					//Expand reallocate headroom for sk_buff
					if(pskb_expand_head(skb, max_headroom - skb_headroom(skb), 0, GFP_ATOMIC))
					{
						printk(KERN_INFO "Unable to expand sk_buff\n");
						return NF_DROP;
					}
				}

				/*
				 *	Push down and install the IPIP header.
				 */
				old_iph=(struct iphdr *)skb_network_header(skb);				
				skb_push(skb, sizeof(struct iphdr));
				skb_reset_network_header(skb);
				
				//I am not sure skb_make_writable is necessary or not
				if(!skb_make_writable(skb,sizeof(struct iphdr)))
                {
					printk(KERN_INFO "Not writable\n");
                    return NF_DROP;
                }

				iph=(struct iphdr *)skb_network_header(skb);
				iph->version=4;
				iph->ihl=sizeof(struct iphdr)>>2;
				iph->tot_len=htons(ntohs(old_iph->tot_len)+sizeof(struct iphdr));
				iph->id=old_iph->id;
				iph->frag_off=old_iph->frag_off;
				iph->protocol=IPPROTO_IPIP;
				iph->tos=old_iph->tos;
				iph->daddr=old_iph->daddr;
				iph->saddr=old_iph->saddr;
				iph->ttl=old_iph->ttl;
				
				//Calculate checksum
				iph->check=0;
				iph->check=ip_fast_csum(iph,iph->ihl);
			}
		}
	}

	return NF_ACCEPT;
}

//PREROUTING for incoming packets
static unsigned int hook_func_in(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	struct iphdr  *old_iph=(struct iphdr *)skb_network_header(skb);

	//We don't listen on eth0
	if(strcmp(in->name,"eth0")!=0)
	{
		if(old_iph!=NULL)
		{
			//We only deal with ICMP packets
			if(old_iph->protocol==IPPROTO_ICMP)
			{
				skb_pull(skb,sizeof(struct iphdr));
				skb_reset_network_header(skb);
				skb->transport_header+=sizeof(struct iphdr);
			}
		}
	}
	return NF_ACCEPT;
}

//Called when module loaded using 'insmod'
int init_module()
{
	//POSTROUTING
	nfho_outgoing.hook = hook_func_out;                   //function to call when conditions below met
	nfho_outgoing.hooknum = NF_INET_POST_ROUTING;         //called in post_routing
	nfho_outgoing.pf = PF_INET;                           //IPV4 packets
	nfho_outgoing.priority = NF_IP_PRI_FIRST;             //set to highest priority over all other hook functions
	nf_register_hook(&nfho_outgoing);                     //register hook
	
	//PREROUTING
	nfho_incoming.hook = hook_func_in;                    //function to call when conditions below met
	nfho_incoming.hooknum = NF_INET_PRE_ROUTING;          //called in pre_routing
	nfho_incoming.pf = PF_INET;                           //IPV4 packets
    nfho_incoming.priority = NF_IP_PRI_FIRST;             //set to highest priority over all other hook functions
	nf_register_hook(&nfho_incoming);                     //register hook
	
	printk(KERN_INFO "Registering Netfilter Hook\n");
	return 0;
}

//Called when module unloaded using 'rmmod'
void cleanup_module()
{
	nf_unregister_hook(&nfho_outgoing); 
	nf_unregister_hook(&nfho_incoming); 

	printk(KERN_INFO "Unregistering Netfilter Hook\n");
}