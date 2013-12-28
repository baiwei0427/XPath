#ifndef RULE_H
#define RULE_H

#include <linux/types.h>
#include <asm/byteorder.h>

//struct of Action
//In this version of RoX, we support three thinks of operations
// 1.Destination Network Address Transformation (DNAT)
// 2.Marking mark of skbuff (For Linux Traffic Control)
// 3.Makring DSCP £¨For QoS in switch) 
struct Action{

	__be32 new_ip;  //New destination ip address need to be transfered to
	__u32 mark;     //Generic packet mark of skbuff
	__u8 dscp;      //Different Service Code Point value (top 6 bits of ToS)

};

//struct of Rule
struct Rule{

	__u8 direction;//direction of packets, incoming (1) or outgoing (2)

	//Following 5 tuples defines a flow
	__be32 src_ip;    //souce ip address 
	__be32 dst_ip;    //destination ip address
	__u8 protocol;    //ip_header->protocol tcp:6 udp:17 icmp:1
	__be16 src_port;  //source tcp/udp port
	__be16 dst_port;  //destination tcp/udp port

	struct Action a;       //action for this flow
};

#endif