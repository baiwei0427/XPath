#include "rox.h"    

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <sys/types.h>


/*struct Rule{
	char*  dev;            //NIC device name (cannot be null)
	__be32 src_ip;         //souce ip address 
	__be32 dst_ip;		   //destination ip address
	unsigned int protocol; //ip_header->protocol tcp:6 udp:17 icmp:1
	unsigned int src_port; //source tcp/udp port
	unsigned int dst_port; //destination tcp/udp port
	__be32 new_ip;         //new ip address need to be transfered to
	int mark;              //new mark
};*/

main()
{
	int file_desc;
	int ret;
	struct Rule r;
	file_desc = open(DEVICE_FILE_NAME, 0);
	if (file_desc < 0) {
		printf ("Can't open device file: %s\n", DEVICE_FILE_NAME);
		exit(-1);
	}
	printf("IOCTL_ROX_INSERT_RULE:%lu\n",IOCTL_ROX_INSERT_RULE);

	r.dev=malloc(sizeof(char)*10);
	memset(r.dev,0,10);
	r.dev="eth0";
	r.src_ip=0x00000056;
	r.dst_ip=0x00000012;
	r.protocol=6;
	r.src_port=80;
	r.dst_port=53;
	r.new_ip=0x00000034;
	r.mark=0x01;
	ret=ioctl(file_desc,IOCTL_ROX_INSERT_RULE,&r);
	close(file_desc);

}