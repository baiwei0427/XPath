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

MODULE_LICENSE("GPL");
 
MODULE_DESCRIPTION("RoX Network Address Transfer");
 
MODULE_AUTHOR("BAI Wei wbaiab@ust.hk");


//Outgoing packets POSTROUTING
static struct nf_hook_ops nfho_outgoing;
//Incoming packets PREROUTING
static struct nf_hook_ops nfho_incoming;

//PREROUTING
unsigned int hook_func_in(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	//printk(KERN_INFO "Incoming device: %s\n",in->name);
	//Incoming natwork device eth1
	struct iphdr *ip_header;
	//unsigned int dst_ip;
	//unsigned int path_id_incoming=0x7938A8C0; //192.168.56.121 0x7838A8C0 
	unsigned int ip_addr_incoming=0x6538A8C0; //192.168.56.101 0x6438A8C0
	//Only listen on eth1
	//if(strcmp(in->name,"eth1")==0)
	//{
		ip_header=(struct iphdr *)skb_network_header(skb);
		//IP packet
		//if(ip_header!=NULL)
		//{
			//dst_ip = (unsigned int)ip_header->daddr;
			//Incoming packet path_id->ip_addr
			//if(dst_ip==path_id_incoming)
			//{
				ip_header->daddr=ip_addr_incoming;
				//Recalculate ip header checksum
				ip_header->check=0;
				ip_header->check=ip_fast_csum(ip_header,ip_header->ihl);	
			//}
		//}
	//}*/
	return NF_ACCEPT;
}

//POSTROUTING
unsigned int hook_func_out(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	//printk(KERN_INFO "Outgoing device: %s\n",out->name);
	struct iphdr *ip_header;
	//unsigned int dst_ip;
    unsigned int path_id_outgoing=0x7838A8C0; //192.168.56.120 0x7938A8C0 
	//unsigned int ip_addr_outgoing=0x6438A8C0; //192.168.56.100 0x6538A8C0 
	//Only listen on eth1
	//if(strcmp(out->name,"eth1")==0)
	//{
		ip_header=(struct iphdr *)skb_network_header(skb);
		//IP packet
		//if(ip_header!=NULL)
		//{
			//dst_ip = (unsigned int)ip_header->daddr;
			//Outoging packet ip_addr->path_id
			//if(dst_ip==ip_addr_outgoing)
			//{
				ip_header->daddr=path_id_outgoing;
				//Recalculate ip header checksum
				ip_header->check=0;
				ip_header->check=ip_fast_csum(ip_header,ip_header->ihl);	
			//}
		//}
	//}
	return NF_ACCEPT;
}


//Called when module loaded using 'insmod'
int init_module()
{
  printk(KERN_INFO "Hook Register\n");
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

  return 0;                                    //return 0 for success
}

//Called when module unloaded using 'rmmod'
void cleanup_module()
{
  printk(KERN_INFO "Hook Unregister\n");
  nf_unregister_hook(&nfho_incoming);
  nf_unregister_hook(&nfho_outgoing);                     //cleanup ¨C unregister hook

}