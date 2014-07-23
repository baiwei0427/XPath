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

static void usage(char* program)
{
	printf("Usage of this program\n");
    printf("%s [print]\n",program);    
    printf("%s [insert|delete|update] [direction] [src_ip] [dst_ip] [protocol] [src_port] [dst_port] [new_ip] [mark] [DSCP]\n",program);
}

main(int argc, char *argv[])
{

		if(argc==2)
		{
			 int file_desc;
             int ret;
             char cmd[7]={0};
                
             //Open device file "/dev/rox"
             file_desc = open(DEVICE_FILE_NAME, 0);
             if (file_desc < 0) 
			 {
				printf ("Can't open device file: %s\n", DEVICE_FILE_NAME);
                exit(-1);
             }
			 strncpy(cmd,argv[1],5);
			 if(strcmp(cmd,"print")==0)
			 {
				ret=ioctl(file_desc,IOCTL_ROX_PRINT_RULE,NULL);
			 }
			 else
			 {
				usage(argv[0]);
			 }
			 close(file_desc);
		}
        else if(argc==11)
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
				/*
				 * Our program requires 10 parameters
                 * 1.command: insert delete update print
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
                r.a.dscp=(unsigned int)strtol(argv[10], NULL, 10);
                
                if(strcmp(cmd,"insert")==0){
                
                        ret=ioctl(file_desc,IOCTL_ROX_INSERT_RULE,&r);

                } else if(strcmp(cmd,"delete")==0) {

                        ret=ioctl(file_desc,IOCTL_ROX_DELETE_RULE,&r);
                
                } else if(strcmp(cmd,"update")==0) {
                
                        ret=ioctl(file_desc,IOCTL_ROX_UPDATE_RULE,&r);
                } else {
					
						usage(argv[0]);
                }
                close(file_desc);
        } 
		else 
		{
			usage(argv[0]);
        }
}