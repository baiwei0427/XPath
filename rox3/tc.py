import os
import sys
import string

#This function is to clear all tc rules in a specific network interface
def clear(intf): 
	os.system('tc qdisc del dev '+intf+' root')

	
#This function is to initialize a htb qdisc in a specific network interface'''	
def init(intf):
	
	#Initialize a root of htb (parent 1:0)
	os.system('tc qdisc add dev '+intf+' root handle 1: htb default 1')
	#Create de default class 1:1 without traffic shaping (1Gbps)
	os.system('tc class add dev '+intf+' parent 1:0 classid 1:1 htb rate 1000mbit ceil 1000mbit prio 1 mtu 9000')

	
#This function is to add tc class and filter into a htb qdisc
def add(intf, classid, rate, nfmark):
	#Add new tc class
	os.system('tc class add dev '+intf+' parent 1:0 classid 1:'+classid+' htb rate '+rate+'mbit ceil '+rate+'mbit prio 1 mtu 9000')
	#Add related filter
	os.system('tc filter add dev '+intf+' parent 1:0 protocol ip prio 1 handle '+nfmark+' fw flowid 1:'+classid)

	
#This function is to del tc class and filter from a htb qdisc
def delete(intf, classid, nfmark):
	#Delte filter
	os.system('tc filter del dev '+intf+' parent 1:0 protocol ip prio 1 handle '+nfmark+' fw flowid 1:'+classid)
	#Delte tc class
	os.system('tc class del dev '+intf+' parent 1:0 classid 1:'+classid)
	
	
	
#This function is to change rate
def change(intf, classid, rate):
	os.system('tc class change dev '+intf+' parent 1:0 classid 1:'+classid+' htb rate '+rate+'mbit ceil '+rate+'mbit prio 1 mtu 9000')
	
	
#This function is to print usage of this script
def usage():
	sys.stderr.write('Usage of this script\n')
	sys.stderr.write('tc.py init [intf]\n')
	sys.stderr.write('tc.py clear [intf]\n')
	sys.stderr.write('tc.py add [intf] [classid] [rate] [nfmark]\n')
	sys.stderr.write('tc.py change [intf] [classid] [rate]\n')
	sys.stderr.write('tc.py del [intf] [classid] [nfmark]\n')
	
#main function
if len(sys.argv)==3:
	if(sys.argv[1]=='init'):
		init(sys.argv[2])
	elif(sys.argv[1]=='clear'):
		clear(sys.argv[2])
	else:
		usage()
elif len(sys.argv)==6:
	if(sys.argv[1]=='add'):
		add(sys.argv[2],sys.argv[3],sys.argv[4],sys.argv[5])
	else:
		usage()
elif len(sys.argv)==5:
	if(sys.argv[1]=='change'):
		change(sys.argv[2],sys.argv[3],sys.argv[4])
	elif(sys.argv[1]=='del'):
		delete(sys.argv[2],sys.argv[3],sys.argv[4])
	else:
		usage()
else:
	usage()