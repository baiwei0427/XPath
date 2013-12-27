import os
import sys
import string

#This function is to start iperf server to receive data 
def start_server(port):
	os.system('iperf -s -p '+port+' -D\n')
	
#This function is to print usage of this script
def usage():
	sys.stderr.write('Usage of this script:\n')
	sys.stderr.write('server.py [server port]\n')

#main function
if len(sys.argv)==2:
	start_server(sys.argv[1])
else:
	usage()