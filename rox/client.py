import os
import sys
import string

#This function is to start iperf client to send data 
def start_client(server,port,size,interval,file):
	os.system('nohup iperf -c '+server+' -p '+port+' -i '+interval+' -n '+size+'MB >'+file+' &\n');
	
#This function is to print usage of this script
def usage():
	sys.stderr.write('Usage of this script:\n')
	sys.stderr.write('client.py [server IP address] [server TCP port] [flow size(MB)] [time interval(Second)] [file]\n')

#main function
if len(sys.argv)==6:
	start_client(sys.argv[1],sys.argv[2],sys.argv[3],sys.argv[4],sys.argv[5])
else:
	usage()
	