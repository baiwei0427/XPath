#ifndef ROX_H
#define ROX_H

#include <linux/ioctl.h>
#include <linux/param.h>

#define MAJOR_NUM 100   //major number of device
#define HASH_RANGE 256 
#define QUEUE_SIZE 256


//struct for a single Rule
//Rule is a 5 tupe of flow (srcip,dstip,srcport,dstport,protocol) in a specific network device (e.g. eth0)
//If the value of some section is zero, that represents *
struct Rule{
	char*  dev;            //NIC device name (cannot be null)
	unsigned int src_ip;   //souce ip address 
	unsigned int dst_ip;   //destination ip address
	unsigned int protocol; //ip_header->protocol tcp:6 udp:17 icmp:1
	unsigned int src_port; //source tcp/udp port
	unsigned int dst_port; //destination tcp/udp port
	unsigned int new_ip;   //new ip address need to be transfered to
	int mark;              //new mark
};

/*typedef struct {
	Rule r;              //Rule in a link node
	RuleNode *next;      //RuleNode pointer to next rule
} RuleNode;

typedef struct {
	int len;             //Length of this RuleList
	RuleNode *head;      //Head pointer 
} RuleList;

typedef struct {

	RuleList *list;     //Array for RuleList

} RuleHashTable;

static int hash(Rule r)
{
	return (0%HASH_RANGE);
}*/

/*
 * _IOR means that we're creating an ioctl command
 * number for passing information from a user process
 * to the kernel module.
 *
 * The first arguments, MAJOR_NUM, is the major device 
 * number we're using.
 *
 * The second argument is the number of the command 
 * (there could be several with different meanings).
 *
 * The third argument is the type we want to get from 
 * the process to the kernel.
 */

#define IOCTL_ROX_INSERT_RULE _IOR(MAJOR_NUM, 0, struct Rule*) //Insert a new rule to rules


#define DEVICE_NAME "rox"
#define DEVICE_FILE_NAME "/dev/rox"

#define SUCCESS 0

#endif