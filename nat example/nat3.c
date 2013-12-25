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
#include <linux/time.h>  

MODULE_LICENSE("GPL");
 
MODULE_DESCRIPTION("RoX Network Address Transfer");
 
MODULE_AUTHOR("BAI Wei wbaiab@ust.hk");


//Outgoing packets POSTROUTING
static struct nf_hook_ops nfho_outgoing;
//Incoming packets PREROUTING
static struct nf_hook_ops nfho_incoming;


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
	//printk(KERN_INFO "Incoming device: %s\n",in->name);
	//Incoming natwork device eth1
	//struct timeval tv_start; //start time
	//struct timeval tv_end;   //end time
	struct iphdr *ip_header;
	__be32 dst_ip;
	__be32 new_ip;
	
	//We don't listen on eth0
	if(strcmp(in->name,"eth0")!=0)
	{
		//do_gettimeofday(&tv_start);
		//printk(KERN_INFO "Incoming packer from %s\n",in->name);
		ip_header=(struct iphdr *)skb_network_header(skb);
		if(!skb_make_writable(skb,sizeof(*ip_header)))
		{
			printk(KERN_INFO "Not writable\n");
			return NF_ACCEPT;
		}

		//IP packet
		if(ip_header!=NULL)
		{
			dst_ip = ip_header->daddr;
			//printk(KERN_INFO "Incoming %x\n", dst_ip);
			//Incoming packet path_id->ip_addr
			//if(dst_ip==path_id_incoming)
			//{
				//ip_header->daddr=0;
				//ip_header->check=0;
				//ip_header->check=ip_fast_csum(ip_header,ip_header->ihl);
			switch(dst_ip)
			{
				//If dst==192.168.2.220 nat to 192.168.2.200
				case 0XDC02A8C0:new_ip=0XC802A8C0;break;
				//If dst==192.168.2.221 nat to 192.168.2.201
				case 0XDD02A8C0:new_ip=0XC902A8C0;break;
				//Others unchanged
				default: new_ip=dst_ip;break;
			}
			csum_replace4(&ip_header->check, dst_ip,new_ip);
			ip_header->daddr=new_ip;
			//}
		}
		//print_packet(skb);
		//do_gettimeofday(&tv_end);
		//printk(KERN_INFO "Time interval %lu\n",((tv_end.tv_sec-tv_start.tv_sec)*1000000+tv_end.tv_usec-tv_start.tv_usec));
	}
	return NF_ACCEPT;
}

//POSTROUTING for outgoing packets
unsigned int hook_func_out(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	//printk(KERN_INFO "Outgoing device: %s\n",out->name);
	//struct timeval tv_start; //start time
	//struct timeval tv_end;   //end time
	struct iphdr *ip_header;
	__be32 dst_ip;
	__be32 new_ip;

	//We don't listen on eth0
	if(strcmp(out->name,"eth0")!=0)
	{
		//do_gettimeofday(&tv_start);
		//printk(KERN_INFO "Outgoing packer to %s\n",out->name);
		ip_header=(struct iphdr *)skb_network_header(skb);
		if(!skb_make_writable(skb,sizeof(*ip_header)))
		{
			printk(KERN_INFO "Not writable\n");
			return NF_ACCEPT;
		}

		//IP packet
		if(ip_header!=NULL)
		{
			dst_ip =ip_header->daddr;
			//printk(KERN_INFO "Outgoing %x\n", dst_ip);
			//Outoging packet ip_addr->path_id
			//if(dst_ip==ip_addr_outgoing)
			//{
				//ip_header->daddr=0;
				//ip_header->check=0;
				//ip_header->check=ip_fast_csum(ip_header,ip_header->ihl);
			switch(dst_ip)
			{
				//If dst==192.168.2.40 nat to 192.168.2.60
				case 0X2802A8C0:new_ip=0X3C02A8C0;break;
				//If dst==192.168.2.41 nat to 192.168.2.61
				case 0X2902A8C0:new_ip=0X3D02A8C0;break;
				//Others unchanged
				default: new_ip=dst_ip;break;
			}
			csum_replace4(&ip_header->check, dst_ip,new_ip);
			ip_header->daddr=new_ip;
			//}
		}
		//print_packet(skb);
		//do_gettimeofday(&tv_end);
		//printk(KERN_INFO "Time interval %lu\n",((tv_end.tv_sec-tv_start.tv_sec)*1000000+tv_end.tv_usec-tv_start.tv_usec));
	}
	return NF_ACCEPT;
}


//Called when module loaded using 'insmod'
int init_module()
{
  printk(KERN_INFO "NAT3 Hook Register\n");

  
  //PREROUTING
  nfho_incoming.hook = hook_func_in;                    //function to call when conditions below met
  nfho_incoming.hooknum = NF_INET_PRE_ROUTING;             //called in pre_routing
  nfho_incoming.pf = PF_INET;                           //IPV4 packets
  nfho_incoming.priority = NF_IP_PRI_FIRST;             //set to highest priority over all other hook functions
  nf_register_hook(&nfho_incoming);                     //register hook*/

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
  printk(KERN_INFO "NAT3 Hook Unregister\n");
  nf_unregister_hook(&nfho_incoming);
  nf_unregister_hook(&nfho_outgoing);                     //cleanup ¨C unregister hook
}
