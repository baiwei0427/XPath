import socket 
import sys

def usage():
	sys.stderr.write('Usage of this script:\n')
	sys.stderr.write('controller.py [IP address] [cmd1:cmd2:...:cmdN]\n')

if len(sys.argv)!=3:
	usage()
else:
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((sys.argv[1], 10086))
	s.send(sys.argv[2])
	data=s.recv(1024)
	s.close()