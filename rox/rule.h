#ifndef RULE_H
#define RULE_H

//struct of Action
struct Action{

	unsigned int new_ip;   //new destination ip address need to be transfered to
	unsigned int mark;     //nfmark

};

//struct of Rule
struct Rule{

	unsigned int direction;//direction of packets, incoming (1) or outgoing (2)

	//Following 5 tuples defines a flow
	unsigned int src_ip;   //souce ip address 
	unsigned int dst_ip;   //destination ip address
	unsigned int protocol; //ip_header->protocol tcp:6 udp:17 icmp:1
	unsigned int src_port; //source tcp/udp port
	unsigned int dst_port; //destination tcp/udp port

	struct Action a;       //action for this flow
};

#endif