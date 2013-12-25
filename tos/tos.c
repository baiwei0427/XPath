#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h>
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
#include <linux/types.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RoX ToS Marking Module");
MODULE_AUTHOR("BAI Wei wbaiab@ust.hk");

//Outgoing packets POSTROUTING
static struct nf_hook_ops nfho_outgoing;
//ToS value you want to mark
static __u8 tos_value;

//POSTROUTING
unsigned int hook_func_out(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	struct iphdr *ip_header;   //ip header struct
	struct tcphdr *tcp_header; //tcp header struct
	
	//Get IP header
	ip_header=(struct iphdr *)skb_network_header(skb);

	//The packet is not a IP packet (e.g. ARP or others)
	if (!ip_header)
	{
		return NF_ACCEPT;
	}
	
	//The packet is not writable, we can't change its ToS value
	if(!skb_make_writable(skb,sizeof(*ip_header)))
	{
		printk(KERN_INFO "Not writable\n");
		return NF_ACCEPT;
	}

	//We just care about iperf data traffic (TCP dst port=5001)
	if(ip_header->protocol==IPPROTO_TCP) 
	{
		tcp_header = (struct tcphdr *)((__u32 *)ip_header+ ip_header->ihl);
		unsigned int dst_port=htons((unsigned short int) tcp_header->dest);
		//TCP dst port=5001
		if(dst_port==5001)
		{
			//Mark new ToS value
			ip_header->tos=tos_value;
			//Recalculate IP header checksum
			ip_header->check=0;
			ip_header->check=ip_fast_csum(ip_header,ip_header->ihl);		
		}
	}
	return NF_ACCEPT;
}

//Called when module loaded using 'insmod'
int init_module()
{
	printk(KERN_INFO "ToS Module Register\n");
	//ToS value
	tos_value=0x20;

	//POSTROUTING
	nfho_outgoing.hook = hook_func_out;                   //function to call when conditions below met
	nfho_outgoing.hooknum = NF_INET_POST_ROUTING;         //called in post_routing
	nfho_outgoing.pf = PF_INET;                           //IPV4 packets
	nfho_outgoing.priority = NF_IP_PRI_FIRST;             //set to highest priority over all other hook functions
	nf_register_hook(&nfho_outgoing);                     //register hook

	return 0;                                    
}

//Called when module unloaded using 'rmmod'
void cleanup_module()
{
  printk(KERN_INFO "ToS Module Unregister\n");
  nf_unregister_hook(&nfho_outgoing);                     //cleanup ¨C unregister hook

}