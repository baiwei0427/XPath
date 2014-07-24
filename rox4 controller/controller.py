import socket 
import sys

def usage():
	sys.stderr.write('Usage of this script:\n')
	sys.stderr.write('controller.py [IP address] [cmd1:cmd2:...:cmdN]\n')
	

#This array is used to transfer Application IP address to Control IP address
#arr[APP IP address]=Control IP address
arr={\
	'192.168.16.1':'192.168.1.40',\
	'192.168.16.2':'192.168.1.41',\
	'192.168.16.3':'192.168.1.42',\
	'192.168.13.1':'192.168.1.43',\
	'192.168.13.2':'192.168.1.44',\
	'192.168.13.3':'192.168.1.45',\
	'192.168.12.1':'192.168.1.46',\
	'192.168.12.2':'192.168.1.47',\
	'192.168.12.3':'192.168.1.48',\
	'192.168.11.1':'192.168.1.49',\
	'192.168.11.2':'192.168.1.50',\
	'192.168.11.3':'192.168.1.51',\
	'192.168.14.1':'192.168.1.52',\
	'192.168.14.2':'192.168.1.53',\
	'192.168.14.3':'192.168.1.54',\
	'192.168.15.1':'192.168.1.55',\
	'192.168.15.2':'192.168.1.56',\
	'192.168.15.3':'192.168.1.57'
	}

if len(sys.argv)!=3:
	usage()
else:
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((arr[sys.argv[1]], 10086))
	s.send(sys.argv[2])
	data=s.recv(1024)
	s.close()