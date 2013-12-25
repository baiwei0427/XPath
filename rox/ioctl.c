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
 */
	if(argc==10)
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
		r.direction=(unsigned int)strtol(argv[2], NULL, 10);
		inet_pton(AF_INET,argv[3],&(r.src_ip));
		inet_pton(AF_INET,argv[4],&(r.dst_ip));
		//r.src_ip=(unsigned int)strtol(argv[2], NULL, 10);
		//r.dst_ip=(unsigned int)strtol(argv[3], NULL, 10);
		r.protocol=(unsigned int)strtol(argv[5], NULL, 10);
		r.src_port=(unsigned int)strtol(argv[6], NULL, 10);
		r.dst_port=(unsigned int)strtol(argv[7], NULL, 10);

		inet_pton(AF_INET,argv[8],&(r.a.new_ip));
		//r.a.new_ip=(unsigned int)strtol(argv[7], NULL, 10);
		r.a.mark=(unsigned int)strtol(argv[9], NULL, 10);
		
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
		
		printf("%s [insert|delete|update] [direction] [src_ip] [dst_ip] [protocol] [src_port] [dst_port] [new_ip] [mark]\n",argv[0]);
		//printf("No enough parameters\n");

	}
}

/*
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
	//printf("IOCTL_ROX_INSERT_RULE:%lu\n",IOCTL_ROX_INSERT_RULE);

	r.src_ip=20;
	r.dst_ip=21;
	r.protocol=6;
	r.src_port=80;
	r.dst_port=53;
	r.a.new_ip=34;
	r.a.mark=1;
	//Insert a new rule
	ret=ioctl(file_desc,IOCTL_ROX_INSERT_RULE,&r);

	r.src_ip=21;
	r.dst_ip=18;
	r.protocol=17;
	r.src_port=80;
	r.dst_port=23;
	r.a.new_ip=34;
	r.a.mark=1;
	//Insert another new rule
	ret=ioctl(file_desc,IOCTL_ROX_INSERT_RULE,&r);

	//Delete the second rule
	ret=ioctl(file_desc,IOCTL_ROX_DELETE_RULE,&r);

	r.src_ip=20;
	r.dst_ip=21;
	r.protocol=6;
	r.src_port=80;
	r.dst_port=53;
	r.a.new_ip=37;
	r.a.mark=9;
	//Update the first rule
	ret=ioctl(file_desc,IOCTL_ROX_UPDATE_RULE,&r);

	close(file_desc);

}*/