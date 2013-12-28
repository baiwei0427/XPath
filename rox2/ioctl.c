#include "rox.h"

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>


main(int argc, char *argv[])
{
/*
 * Our program requires 8 parameters
 * 1.command: insert delete update
 * 2.direction
 * 3.src_ip
 * 4.dst_ip
 * 5.protocol
 * 6.src_port
 * 7.dst_port
 * 8.new_ip
 * 9.mark
 * 10.dscp
 */

	if(argc==11)
	{
		int file_desc;
		int ret;
		struct Rule r;
		char cmd[7]={0};
		
		//Open device file "/dev/rox"
		file_desc = open(DEVICE_FILE_NAME, 0);
		if (file_desc < 0) {
			printf ("Can't open device file: %s\n", DEVICE_FILE_NAME);
			exit(-1);
		}

		strncpy(cmd,argv[1],6);
		r.direction=(__u8)strtol(argv[2], NULL, 10);

		//Change IP address string to network binary bytes
		inet_pton(AF_INET,argv[3],&(r.src_ip));
		inet_pton(AF_INET,argv[4],&(r.dst_ip));

		r.protocol=(__u8)strtol(argv[5], NULL, 10);

		//Change 
		r.src_port=(__be16)htons((unsigned short int)strtol(argv[6], NULL, 10));
		r.dst_port=(__be16)htons((unsigned short int)strtol(argv[7], NULL, 10));

		inet_pton(AF_INET,argv[8],&(r.a.new_ip));
		//r.a.new_ip=(unsigned int)strtol(argv[7], NULL, 10);
		r.a.mark=(__u32)strtol(argv[9], NULL, 10);
		r.a.dscp=(__u8)strtol(argv[10], NULL, 10);
		
		if(strcmp(cmd,"insert")==0){
		
			ret=ioctl(file_desc,IOCTL_ROX_INSERT_RULE,&r);

		} else if(strcmp(cmd,"delete")==0) {

			ret=ioctl(file_desc,IOCTL_ROX_DELETE_RULE,&r);
		
		} else if(strcmp(cmd,"update")==0) {
		
			ret=ioctl(file_desc,IOCTL_ROX_UPDATE_RULE,&r);

		} else {
		
			printf("Invalid input parameters. We only support \"insert\", \"delete\" and \"update\"\n");

		}
		close(file_desc);
	} else {
		
		printf("%s [insert|delete|update] [direction] [src_ip] [dst_ip] [protocol] [src_port] [dst_port] [new_ip] [mark] [DSCP]\n",argv[0]);

	}
}