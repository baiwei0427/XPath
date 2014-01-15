import os
import sys
import string

#This script is to start iperf server to receive data 
#In RoX 1.2, we support UDP

#This function is to start iperf TCP server to receive data 
def start_TCP_server(port):
	os.system('iperf -s -p '+port+' -D\n')

#This function is to start iperf UDP server to receive data
#The only difference between TCP and UDP is the parameter '-u' (UDP)
def start_UDP_server(port):
	os.system('iperf -s -u -p '+port+' -D\n')

#This function is to print usage of this script
def usage():
	sys.stderr.write('Usage of this script:\n')
	sys.stderr.write('server.py [protocol (TCP 6/UDP 17)] [server port]\n')

#main function
if len(sys.argv)==3:
	#start TCP server
	if int(sys.argv[1])==6:
		start_TCP_server(sys.argv[2])
	elif int(sys.argv[1])==17:
		start_UDP_server(sys.argv[2])
	else:
		usage()
else:
	usage()