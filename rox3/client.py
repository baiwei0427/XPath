import os
import sys
import string

#This script is to start iperf client to send data 
#In RoX 1.1, we change minimum unit of traffic from MB to KB, giving a better support to mice flows 
#In RoX 1.2, we support UDP

#This function is to start iperf TCP client to send data
def start_TCP_client(server,port,size,interval,file):
	os.system('nohup iperf -c '+server+' -p '+port+' -i '+interval+' -n '+size+'KB >'+file+' &\n');
	
#This function is to strat iperf UDP client to send data
#By default, we use 1Gigabit NIC. So default sending rate of UDP is 1000Mbps
def start_UDP_client(server,port,size,interval,file):
	os.system('nohup iperf -c '+server+' -u -p '+port+' -i '+interval+' -n '+size+'KB -b 1000M>'+file+' &\n');
	
#This function is to print usage of this script
def usage():
	sys.stderr.write('Usage of this script:\n')
	sys.stderr.write('client.py [receiver IP address] [protocol (TCP 6/UDP 17)] [server port] [flow size(KB)] [interval(Second)] [file]\n')

#main function
if len(sys.argv)==7:
	if int(sys.argv[2])==6:
		start_TCP_client(sys.argv[1],sys.argv[3],sys.argv[4],sys.argv[5],sys.argv[6])
	elif int(sys.argv[2])==17:
		start_UDP_client(sys.argv[1],sys.argv[3],sys.argv[4],sys.argv[5],sys.argv[6])
	else:
		usage()
else:
	usage()
	